//
// lib.rs
// Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
// Distributed under terms of the BSD (2-clause) license.
//
// Created on: 24 May 2017 19:26:33 +0200 (CEST)
//

#![feature(lang_items, core_intrinsics)]
#![no_main]
#![no_std]

extern crate rlibc;

use core::intrinsics;

#[cfg_attr(target_arch = "x86", path = "arch/x86/mod.rs")]
#[macro_use]
mod arch;

#[no_mangle]
pub extern "C" fn kmain() {
  arch::early_init();
  arch::init_output();

  println!("hello, world says the kernel!");

  loop {}
}

#[lang = "eh_personality"]
#[no_mangle]
pub extern fn rust_eh_personality() {
}

#[lang = "eh_unwind_resume"]
#[no_mangle]
pub extern fn rust_eh_unwind_resume() {
}

#[lang = "panic_fmt"]
#[no_mangle]
pub extern fn rust_begin_panic(_msg: core::fmt::Arguments,
                               _file: &'static str,
                               _line: u32) -> ! {
  unsafe {
    intrinsics::abort()
  }
}

/*
 * vi: ts=2 sw=2 expandtab
 */

