//
// build.rs
// Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
// Distributed under terms of the BSD (2-clause) license.
//
// Created on: 06 Jan 2017 22:30:24 +0100 (CET)
//

extern crate nasm_rs;

use std::env;

fn main() {
  let out_dir = env::var("OUT_DIR").unwrap();

  nasm_rs::compile_library("entry.o", &["entry.asm"]);

  println!("cargo:rustc-link-search=native={}", out_dir);
  println!("cargo:rustc-link-lib=static=entry");
  println!("cargo:rerun-if-changed=/src/asm/boot.asm");
}

/*
 * vi: ts=2 sw=2 expandtab
 */

