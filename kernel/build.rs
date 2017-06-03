//
// build.rs
// Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
// Distributed under terms of the BSD (2-clause) license.
//
// Created on: 21 May 2017 21:08:29 +0200 (CEST)
//

extern crate nasm_rs;
extern crate glob;

use std::env;

#[allow(non_camel_case_types)]
#[derive(Debug)]
enum Arch {
  x86,
  unknown,
}

fn main() {
  let target_triple = env::var("TARGET").unwrap();
  let arch = if target_triple.starts_with("i686") {
    Arch::x86
  } else {
    Arch::unknown
  };

  for file in glob::glob("src/*.asm").unwrap().chain(
              glob::glob(format!("src/arch/{:?}/*.asm", arch).as_ref()).unwrap()) {
    match file {
      Ok(path) => {
        let libname = path.file_stem().unwrap().to_str().unwrap();
        let path = path.to_str().unwrap();

        let nasm_args = match arch {
          Arch::x86 => ["-f elf32"],
          Arch::unknown => [""],
        };

        nasm_rs::compile_library_args(format!("lib{}.a", libname).as_ref(), &[path], &nasm_args);

        println!("cargo:rustc-link-lib=static={}", libname);
        println!("cargo:rerun-if-changed={}", path);
      },
      _ => (),
    }
  }

  println!("cargo:rerun-if-changed=src/arch/{:?}/layout.ld", arch);
  println!("cargo:link-arg=-Tsrc/arch/{:?}/layout.ld", arch);

  ()
}

/*
 * vi: ts=2 sw=2 expandtab
 */

