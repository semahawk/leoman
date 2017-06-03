//
// mod.rs
// Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
// Distributed under terms of the BSD (2-clause) license.
//
// Created on: 03 Jun 2017 18:37:58 +0200 (CEST)
//

pub mod vga;

pub fn early_init() {
  unsafe {
    let vga = 0xb8000 as *mut u32;

    *vga.offset(0) = 0x3f383f78;
    *vga.offset(1) = 0x3f36;
  };
}

pub fn init_output() {

}

macro_rules! println {
  () => (($crate::arch::vga::VGA.write_char('i')));
  ($i:expr) => (($crate::arch::vga::VGA.write_str($i)));
}

/*
 * vi: ts=2 sw=2 expandtab
 */

