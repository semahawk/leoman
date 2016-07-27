/*
 *
 * sar.c
 *
 * Created at:  Fri 28 Nov 22:03:01 2014 22:03:01
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>

/* this is flippin' hackish */
#include "../include/kernel/sar.h"

#define PALIGN(addr) ((((uint32_t)(addr)) + 4096 - 1) & ~(4096 - 1))

struct cmd {
  char ch;
  int (*func)(int, char *[], const struct cmd *);
  const char *usage;
};

static int create(int, char *[], const struct cmd *);
static int extract(int, char *[], const struct cmd *);
static int list(int, char *[], const struct cmd *);

static const struct cmd commands[] = {
  { 'c', create,  "<archive> <files>" },
  { 'e', extract, "<archive>" },
  { 'l', list,    "<archives>" },
  { '\0', NULL, NULL }
};

static const struct cmd *const cmd_lookup(char ch)
{
  int i;

  for (i = 0; commands[i].func != NULL; i++)
    if (commands[i].ch == ch)
      return &commands[i];

  return NULL;
}

static int create(int argc, char *argv[], const struct cmd *cmd)
{
  /* {{{ */
  /* the to-be-created archive's file descriptor */
  int archive;
  struct sar_header ahdr;

  int i;
  unsigned total_hdr_size = 0;
  uint32_t last_file_offset;
  uint32_t last_fhdr_addr = sizeof(struct sar_header);
  struct sar_file terminator;

  if (argc < 2){
    fprintf(stderr, "usage: sar c %s\n", cmd->usage);
    return 1;
  }

  archive = open(argv[0], O_WRONLY | O_TRUNC | O_CREAT, 0644);

  /* skip over the archive's name */
  argc--;
  argv++;

  /* set up the archive's header */
  ahdr.magic = SAR_MAGIC;

  /* first off, start with the magic */
  write(archive, &ahdr, sizeof(struct sar_header));

  /* calculate the archive's header total size */
  for (i = 0; i < argc; i++){
    total_hdr_size += sizeof(struct sar_file);
    total_hdr_size += strlen(basename(argv[i])) + 1 /* nul byte */;
  }

  /* make account for the archive's header */
  total_hdr_size += sizeof(struct sar_header);
  /* and for a terminating file header (all zeroes) */
  total_hdr_size += sizeof(struct sar_file);
  /* align the header size to a 4KiB boundary */
  total_hdr_size = PALIGN(total_hdr_size);
  /* next file's contents will come right after the header */
  last_file_offset = total_hdr_size;

  /* write the archive's header */
  for (i = 0; i < argc; i++){
    int fd;
    char *buf;
    struct stat st;
    struct sar_file fhdr;

    /* fetch the file's size to calculate the next file offset */
    if ((fd = open(argv[i], O_RDONLY)) == -1){
      perror(argv[i]);
      /* try the next file */
      continue;
    }

    if (stat(argv[i], &st) == -1){
      perror(argv[i]);
      /* try the next file */
      continue;
    }

    /* fill in the header */
    fhdr.size    = st.st_size;
    fhdr.offset  = last_file_offset;
    fhdr.namelen = strlen(basename(argv[i])) + 1;

    last_fhdr_addr += sizeof(fhdr) + fhdr.namelen;

    if ((buf = malloc(PALIGN(fhdr.size))) == NULL){
      perror("malloc");
      return 1;
    }

    /* fetch the file's contents */
    if (read(fd, buf, fhdr.size) == -1){
      perror("read");
      return 1;
    }

    /* zero-out the additional aligned area */
    memset(buf + fhdr.size, 0x0, PALIGN(fhdr.size) - fhdr.size);

    /* write the header out to the archive */
    write(archive, &fhdr, sizeof(fhdr));
    /* write the filename right after the header */
    write(archive, basename(argv[i]), fhdr.namelen);
    /* write out the file's contents, at it's offset */
    pwrite(archive, buf, PALIGN(fhdr.size), last_file_offset);

    free(buf);

    last_file_offset += PALIGN(fhdr.size);
  }

  /* write out the terminating file header (all zeroes) */
  memset(&terminator, 0x0, sizeof(terminator));
  pwrite(archive, &terminator, sizeof(terminator), last_fhdr_addr);

  close(archive);

  return 0;
  /* }}} */
}

static int list(int argc, char *argv[], const struct cmd *cmd)
{
  /* {{{ */
  int i;
  int fd;
  char *buf;
  struct stat st;
  struct sar_file *fhdr;

  for (i = 0; i < argc; i++){
    if ((fd = open(argv[i], O_RDONLY)) == -1){
      perror(argv[i]);
      /* try the next one */
      continue;
    }

    /* get the archive's size */
    if (stat(argv[i], &st) == -1){
      perror(argv[i]);
      /* try the next one */
      continue;
    }

    /* make room for the archive's contents */
    if ((buf = malloc(st.st_size)) == NULL){
      perror(argv[i]);
      return 1;
    }

    /* fetch the archive's contents */
    if (read(fd, buf, st.st_size) == -1){
      perror(argv[i]);
      return 1;
    }

    for (fhdr = (struct sar_file *)(buf + sizeof(struct sar_header));
         /* all of the fields have to be zero */
         fhdr->size | fhdr->offset | fhdr->namelen;
         fhdr = (struct sar_file *)((char *)fhdr + sizeof(struct sar_file) + fhdr->namelen)){
      printf("%16s: sz 0x%x (%d), off 0x%x\n", FHDR_FNAME(fhdr), fhdr->size, fhdr->size, fhdr->offset);
    }
  }

  return 0;
  /* }}} */
}

static int extract(int argc, char *argv[], const struct cmd *cmd)
{
  /* {{{ */
  int i;
  int archive;
  char *buf;
  struct stat st;
  struct sar_file *fhdr;

  for (i = 0; i < argc; i++){
    if ((archive = open(argv[i], O_RDONLY)) == -1){
      perror(argv[i]);
      /* try the next one */
      continue;
    }

    /* get the archive's size */
    if (stat(argv[i], &st) == -1){
      perror(argv[i]);
      /* try the next one */
      continue;
    }

    /* make room for the archive's contents */
    if ((buf = malloc(st.st_size)) == NULL){
      perror(argv[i]);
      return 1;
    }

    /* fetch the archive's contents */
    if (read(archive, buf, st.st_size) == -1){
      perror(argv[i]);
      return 1;
    }

    for (fhdr = (struct sar_file *)(buf + sizeof(struct sar_header));
         /* all of the fields have to be zero */
         fhdr->size | fhdr->offset | fhdr->namelen;
         fhdr = (struct sar_file *)((char *)fhdr + sizeof(struct sar_file) + fhdr->namelen)){
      int fd;

      if ((fd = open(FHDR_FNAME(fhdr), O_WRONLY | O_TRUNC | O_CREAT, 0644)) == -1){
        perror(FHDR_FNAME(fhdr));
        free(buf);
        return 1;
      }

      /* create the file by writing to it */
      write(fd, buf + fhdr->offset, fhdr->size);

      close(fd);
    }
  }

  close(archive);

  return 0;
  /* }}} */
}

int main(int argc, char *argv[])
{
  char ch;
  const struct cmd *cmd;

  if (argc < 2){
    fprintf(stderr, "basic usage: sar <command> <additional args>\n\n");
    fprintf(stderr, "commands:\n");
    fprintf(stderr, "  c    create a new archive\n");
    fprintf(stderr, "  l    list files in archive\n");
    return 1;
  }

  ch = argv[1][0];

  if ((cmd = cmd_lookup(ch)) == NULL){
    fprintf(stderr, "sar: unknown command: '%c'\n", ch);
    return 1;
  }

  if (argc == 2){
    fprintf(stderr, "usage: sar %c %s\n", ch, cmd->usage);
    return 1;
  }

  /* skip over the program's name and the command */
  argc -= 2;
  argv += 2;

  return cmd->func(argc, argv, cmd);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

