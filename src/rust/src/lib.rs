extern crate regex;
extern crate libc;

use regex::Regex;
use libc::c_char;
use std::boxed::Box;
use std::ffi::CStr;
use std::slice;
use std::str;
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
    let s = unsafe { 
        let slice = slice::from_raw_parts(p, len as usize);
        str::from_utf8(slice).unwrap()
    };

    let findings = exp.find_iter(s).count();
    Box::into_raw(exp);
    findings as u64
}

#[no_mangle]
pub extern fn regex_free(raw_exp: *mut Regex) {
    unsafe { Box::from_raw(raw_exp) };
}
