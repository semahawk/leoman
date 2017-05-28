#![feature(lang_items, core_intrinsics)]
#![feature(lang_items, start)]
#![no_std]

use core::intrinsics;

fn main() {
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
