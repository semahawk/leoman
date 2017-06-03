//
// vga.rs
// Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
// Distributed under terms of the BSD (2-clause) license.
//
// Created on: 03 Jun 2017 19:52:44 +0200 (CEST)
//

//use core::fmt;
//use core::fmt::Write;

pub struct Vga();

const VGA_BASE: *mut u16 = 0xb8000 as *mut u16;
const MAX_COLS: isize = 25;
const MAX_ROWS: isize = 80;
static mut CURR_X: isize = 0;
static mut CURR_Y: isize = 0;

pub const VGA: Vga = Vga();

impl Vga {
  pub fn write_char(&self, ch: char) {
    unsafe {
      // TODO: get rid of the unsafe
      let idx = CURR_Y * MAX_COLS + CURR_X;

      *VGA_BASE.offset(idx) = 0x3f00 + (ch as u8) as u16;

      CURR_X += 1;
    }
  }

  pub fn write_str(&self, s: &str) {
    for ch in s.chars() {
      self.write_char(ch);
    }
  }
}



/*
 * vi: ts=2 sw=2 expandtab
 */

