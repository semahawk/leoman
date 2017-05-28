//
// build.rs
// Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
// Distributed under terms of the BSD (2-clause) license.
//
// Created on: 21 May 2017 21:08:29 +0200 (CEST)
//

extern crate nasm_rs;
extern crate glob;

fn main() {
  for file in glob::glob("src/*.asm").unwrap().chain(glob::glob("src/arch/**/*.asm").unwrap()) {
    match file {
      Ok(path) => {
        let libname = path.file_stem().unwrap().to_str().unwrap();
        let path = path.to_str().unwrap();
        nasm_rs::compile_library_args(format!("lib{}.a", libname).as_ref(), &[path], &["-f elf32"]);
        println!("cargo:rustc-link-lib=static={}", libname);
        println!("cargo:rerun-if-changed={}", path);
      },
      _ => (),
    }
  }

  println!("cargo:rerun-if-changed=src/arch/{}/layout.ld", cfg!(arch));
  println!("cargo:link-arg=-Tsrc/arch/{}/layout.ld", cfg!(arch));
}

/*
 * vi: ts=2 sw=2 expandtab
 */

