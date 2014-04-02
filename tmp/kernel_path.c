/*
 *
 * kernel_path.c
 *
 * Created at:  Tue 25 Mar 15:51:25 2014 15:51:25
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <string.h>

static char kernel_path[] = "/boot/kernel/kernel";
static unsigned i = 0;

int main(void)
{
  char *p = kernel_path;
  char buff[256] = { 0 };
  char *bp = buff;

  asm (
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );

  printf("load inode 0x02 into the memory\n");

  while (*p++ != '\0'){
    printf("seek%u:\n", i);
    printf("  zero-out the name buffer\n");
    memset(buff, 0x0, 256);
    bp = buff;

    printf("  fetch the next path segment into the name buffer\n");
    for (; *p != '\0' && *p != '/'; p++){
      *bp++ = *p;
    }

    printf("  begin search for '%s'\n", buff);
    printf("  load the first block of the inode which is currently in the memory\n");
    printf("  traverse:\n");
    printf("    if found\n");
    printf("      if last segment (ie. *p == '\\0')\n");
    printf("        if is a file\n");
    printf("          load it's inode in place of the previous one\n");
    printf("          jmp found\n");
    printf("        else\n");
    printf("          print '%s is not a regular file'\n", kernel_path);
    printf("          jmp halt\n");
    printf("      else\n");
    printf("        if is a directory\n");
    printf("          load it's inode in place of the previous one\n");
    printf("          jmp seek%u\n", i + 1);
    printf("        else\n");
    printf("          print '%s is not a directory'\n", buff);
    printf("          jmp halt\n");
    printf("    else\n");
    printf("      if this was the last block\n");
    printf("        print kernel not found!\n");
    printf("        jmp halt\n");
    printf("      else\n");
    printf("        load the next block into the memory\n");
    printf("        jmp traverse\n");
    printf("\n");
    i++;
  }

  printf("found:\n");
  printf("  given the inode structure in the memory, fetch all the data"
      "blocks and store them in memory\n");

  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

