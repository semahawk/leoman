//
// mod.rs
// Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
// Distributed under terms of the BSD (2-clause) license.
//
// Created on: 03 Jun 2017 18:37:58 +0200 (CEST)
//

pub fn early_init() {
  unsafe {
    let vga = 0xb8000 as *mut u32;

    *vga.offset(0) = 0x3f383f78;
    *vga.offset(1) = 0x3f36;
  };
}

/*
 * vi: ts=2 sw=2 expandtab
 */

