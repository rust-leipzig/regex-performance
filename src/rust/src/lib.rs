extern crate regex;
extern crate regress;
extern crate libc;

use regex::bytes::Regex;
use regress::Regex as Regress;
use libc::c_char;
use std::boxed::Box;
use std::ffi::CStr;
use std::slice;
use std::ptr;

#[no_mangle]
pub extern fn regex_new(c_buf: *const c_char) -> *const Regex {
    let c_str: &CStr = unsafe { CStr::from_ptr(c_buf) };
    
    let exp = match Regex::new(c_str.to_str().unwrap()) {
       Ok(val) => Box::into_raw(Box::new(val)),
       Err(_) => ptr::null()
    };

    exp as *const Regex
}

#[no_mangle]
pub extern fn regex_matches(raw_exp: *mut Regex, p: *const u8, len: u64) -> u64 {
    let exp = unsafe { Box::from_raw(raw_exp) };
    let s = unsafe { slice::from_raw_parts(p, len as usize) };

    let findings = exp.find_iter(s).count();
    Box::into_raw(exp);
    findings as u64
}

#[no_mangle]
pub extern fn regex_free(raw_exp: *mut Regex) {
    unsafe { Box::from_raw(raw_exp) };
}

#[no_mangle]
pub extern fn regress_new(c_buf: *const c_char) -> *const Regress {
    let c_str: &CStr = unsafe { CStr::from_ptr(c_buf) };

    // Pretend regress supports the "(?i)" syntax.
    let mut pat = c_str.to_str().unwrap();
    let mut flags = regress::Flags::default();
    if pat.starts_with("(?i)") {
        flags.icase = true;
        pat = &pat["(?i)".len()..]
    }
    let exp = match regress::Regex::newf(pat, flags) {
        Ok(val) => Box::into_raw(Box::new(val)),
        Err(_) => ptr::null(),
    };

    exp as *const Regress
}

#[no_mangle]
pub extern fn regress_matches(raw_exp: *mut Regress, p: *const u8, len: u64) -> u64 {
    let exp = unsafe { Box::from_raw(raw_exp) };
    let s = unsafe {
        let sl = slice::from_raw_parts(p, len as usize);
        std::str::from_utf8_unchecked(sl)
    };

    let findings = exp.find_iter_ascii(s).count();
    Box::into_raw(exp);
    findings as u64
}

#[no_mangle]
pub extern fn regress_free(raw_exp: *mut Regress) {
    unsafe { Box::from_raw(raw_exp) };
}
