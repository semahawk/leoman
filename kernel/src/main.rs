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

use core::intrinsics;

#[no_mangle]
pub extern "C" fn kmain() {
  unsafe {
    let vga = 0xb8000 as *mut u32;

    *vga = 0x3f4b3f4f;
  };

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

