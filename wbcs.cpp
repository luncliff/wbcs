/**
 * @author  luncliff (luncliff@gmail.com)
 *
 * @see     https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-s-wfopen-s#unicode-support
 * @see     https://docs.microsoft.com/en-us/cpp/build/reference/source-charset-set-source-character-set?view=vs-2019
 * @see     https://github.com/google/leveldb/issues/755
 * @see     https://gist.github.com/luncliff/a39287deb3618b24e402839c5db4e74b
 */
#if defined(_MSC_FULL_VER)
#pragma setlocale(".65001")
#endif
#include "wbcs.hpp"

#include <codecvt>
#include <iostream>
#include <stdexcept>

static_assert(__cplusplus >= 201402L, "requires C++ 14 or later");
static_assert(L_tmpnam >= 260, "expect 260 as minimum value of the path limit");

using std::string;
using std::wstring;

errno_t wbcs_w2mb(const wstring& in, string& out) noexcept(false) {
    out.reserve(MB_CUR_MAX * in.length()); /// @see wcsnlen

    mbstate_t state{};
    for (wchar_t wc : in) {
        char mb[8]{}; // ensure null-terminated for UTF-8 (maximum 4 byte)
        const auto len = ::wcrtomb(mb, wc, &state);
        switch (len) {
        case static_cast<size_t>(-1):
            return errno; // EILSEQ
        default:
            out.append(mb, len); // out += string_view{mb, len};
        }
    }
    return 0;
}

errno_t wbcs_mb2w(const string& in, wstring& out) noexcept(false) {
    const auto sz = mblen(in.c_str(), in.length());
    if (sz < 0) // can't process
        return errno;

    // if sz == 0, try best with `mbrtowc`'s implementation
    out.reserve(static_cast<size_t>(sz));
    const char* pos = in.data();
    const char* const end = in.data() + in.length();

    mbstate_t state{};
    wchar_t wc{};
    while (size_t len = ::mbrtowc(&wc, pos, end - pos, &state)) {
        if (len == static_cast<size_t>(-1)) // bad encoding
            return errno;
        if (len == static_cast<size_t>(-2)) // valid but incomplete
            break;                          // nothing to do more
        out.push_back(wc);
        pos += len; // advance [1...n]
    }
    return 0;
}

#if defined(_WIN32)
// errno_t wbcs_empty_locale(std::wistream& s) noexcept(false) {
//     auto converter = new std::codecvt_utf8<wchar_t>{};
//     s.imbue(std::locale(std::locale::empty(), converter));
//     return errno;
// }
errno_t wbcs_empty_locale(std::istream& s) noexcept(false) {
    auto converter = new std::codecvt_utf8<wchar_t>{};
    s.imbue(std::locale(std::locale::empty(), converter));
    return errno;
}

void wbcs_replace(const std::string& in, string& out, //
                  const char* delims, const char* replacement) noexcept {
    static_assert(false, "not implemented");
}

errno_t wbcs_open(FILE** ptr, const wstring& fpath) noexcept {
    if (ptr == nullptr)
        return EINVAL;
    return _wfopen_s(ptr, fpath.c_str(), L"rb, ccs=UNICODE");
}

errno_t wbcs_create(FILE** ptr, const wstring& fpath) noexcept {
    if (ptr == nullptr)
        return EINVAL;
    return _wfopen_s(ptr, fpath.c_str(), L"w+b");
}
errno_t wbcs_append(FILE** ptr, const wstring& fpath) noexcept {
    if (ptr == nullptr)
        return EINVAL;
    return _wfopen_s(ptr, fpath.c_str(), L"a+b");
}
#else
errno_t wbcs_empty_locale(std::wistream& s) noexcept(false) {
    std::locale empty{};
    s.imbue(empty);
    return errno;
}
errno_t wbcs_empty_locale(std::istream& s) noexcept(false) {
    std::locale empty{};
    s.imbue(empty);
    return errno;
}

void wbcs_replace(const std::string& in, string& out, //
                  const char* delims, const char* replacement) noexcept {
    char* pos = const_cast<char*>(in.c_str());
    char* token = nullptr;
    while ((token = strsep(&pos, delims)) != nullptr) {
        out.append(token);
        if (pos) // we have next token
            out.append(replacement);
    }
}

/**
 * @todo length check with `MB_CUR_MAX`
 */
errno_t wbcs_open(FILE** ptr, const wstring& fpath, const char* mode) noexcept {
    if (ptr == nullptr) // ensure not-null before string operation ...
        return EINVAL;

    string path{};
    if (auto ec = wbcs_w2mb(fpath, path))
        return ec;

    // `fopen` fails when there are spaces, replace them ...
    path = wbcs_replace(path, " ", "\x20");
    if ((*ptr = fopen(path.c_str(), mode)) == nullptr)
        return errno;
    return 0;
}

errno_t wbcs_open(FILE** ptr, const std::wstring& fpath) noexcept {
    if (ptr == nullptr) // fail fast
        return EINVAL;
    return wbcs_open(ptr, fpath, "r");
}
errno_t wbcs_create(FILE** ptr, const std::wstring& fpath) noexcept {
    if (ptr == nullptr) // fail fast
        return EINVAL;
    return wbcs_open(ptr, fpath, "w+");
}
errno_t wbcs_append(FILE** ptr, const std::wstring& fpath) noexcept {
    if (ptr == nullptr) // fail fast
        return EINVAL;
    return wbcs_open(ptr, fpath, "a+");
}

#endif

string wbcs_replace(const std::string& in, //
                    const char* delims,
                    const char* replacement) noexcept(false) {
    string out{};
    out.reserve(MB_CUR_MAX * in.length());
    wbcs_replace(in, out, delims, replacement);
    return out;
}
