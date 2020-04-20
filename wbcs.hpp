/**
 * @author  luncliff (luncliff@gmail.com)
 * @brief   functions to support existing <filesystem> modules under WBCS
 * 
 * @see     <experimental/filesystem>
 * @see     <ghc/filesystem.hpp>
 * 
 * @todo    <boost/filesystem.hpp>
 */
#pragma once
#include <clocale>
#include <cstdio>
#include <cwchar>
#include <istream>
#include <locale>
#include <memory>
#include <string>

/**
 * @param   in      `wchar_t` sequence to make MBCS
 * @param   out     `string` to append converted `in`
 * @return  uint32_t `errno` from `wcrtomb`
 * 
 * @see     wcrtomb
 * @see     wcstombs
 * @throw   std::length_error
 * @throw   std::bad_alloc
 */
uint32_t wbcs_w2mb(const std::wstring& in, std::string& out) noexcept(false);

/**
 * @param   in      `char` sequence to make WBCS
 * @param   out     `wstring` to append converted `in`
 * @return  uint32_t `errno` from `mbrtowc`
 * 
 * @see     mbrtowc
 * @see     mbstowcs
 * @throw   std::length_error
 * @throw   std::bad_alloc
 */
uint32_t wbcs_mb2w(const std::string& in, std::wstring& out) noexcept(false);

/**
 * @param in        UTF-8 (multi-byte) string in `char`
 */
std::string wbcs_replace(const std::string& in, //
                         const char* delims = " ",
                         const char* replacement = "\x20") noexcept(false);

/**
 * @see     https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-s-wfopen-s?view=vs-2019
 */
uint32_t wbcs_open(FILE** ptr, const std::wstring& fpath,
                   const char* mode) noexcept;

/**
 * @brief   `fopen` the file with "r"
 */
uint32_t wbcs_open(FILE** ptr, const std::wstring& fpath) noexcept;
/**
 * @brief   `fopen` the file with "w+"
 */
uint32_t wbcs_create(FILE** ptr, const std::wstring& fpath) noexcept;
/**
 * @brief   `fopen` the file with "a+"
 */
uint32_t wbcs_append(FILE** ptr, const std::wstring& fpath) noexcept;

/**
 * @brief   Change the locale of `std::istream` to empty.
 * @details We are in C++ 14. Ignore the `codecvt_utf8` deprecation ...
 *          Notice that the type makes crash when used as a stack variable
 * 
 * @param   s 
 * @return  uint32_t 
 * 
 * @see     https://en.cppreference.com/w/cpp/io/ios_base/getloc
 * @see     https://en.cppreference.com/w/cpp/locale/locale
 */
uint32_t wbcs_empty_locale(std::istream& s) noexcept(false);
uint32_t wbcs_empty_locale(std::wistream& s) noexcept(false);
