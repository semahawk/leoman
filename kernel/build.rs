//
// build.rs
// Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
// Distributed under terms of the BSD (2-clause) license.
//
// Created on: 21 May 2017 21:08:29 +0200 (CEST)
//

extern crate nasm_rs;

fn main() {
  nasm_rs::compile_library_args("libentry.a", &["src/entry.asm", "src/multiboot_header.asm"], &["-f elf32"]);

  println!("cargo:rerun-if-changed=src/entry.asm");
  println!("cargo:rerun-if-changed=src/multiboot_header.asm");

  println!("cargo:rustc-link-lib=static=entry");

  println!("cargo:rerun-if-changed=src/layout.ld");
}

/*
 * vi: ts=2 sw=2 expandtab
 */

