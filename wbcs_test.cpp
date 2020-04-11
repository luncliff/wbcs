#if defined(_MSC_FULL_VER)
#pragma setlocale(".65001")
#endif
#include <boost/ut.hpp>
#include <ghc/filesystem.hpp>

#include <locale>

#include "wbcs.hpp"

namespace testing = boost::ut;
namespace gfs = ghc::filesystem;

using namespace std;
using namespace std::string_literals;

/**
 * @brief Can we change `wstring` to `string`? and vice versa?
 */
void test_unicode_literal(const wchar_t* w, const char* mb) {
    {
        string output{};
        testing::expect(wbcs_w2mb(w, output) == 0);
        testing::expect(output == mb);
    }
    {
        wstring output{};
        testing::expect(wbcs_mb2w(mb, output) == 0);
        testing::expect(output == w);
    }
}

/**
 * @todo https://github.com/microsoft/terminal/blob/master/src/types/convert.cpp
 */
void test_languages(FILE*) {
    using namespace testing;

    "korean"_test = []() {
        const auto w = L"나의 사랑 한글날";
        const auto mb = "나의 사랑 한글날";
        test_unicode_literal(w, mb);
    };
    "russian"_test = []() {
        const auto w = L"Сою́з Сове́тских Социалисти́ческих Респу́блик";
        const auto mb = "Сою́з Сове́тских Социалисти́ческих Респу́блик";
        test_unicode_literal(w, mb);
    };
    "chinese"_test = []() {
        const auto w = L"分久必合 合久必分";
        const auto mb = "分久必合 合久必分";
        test_unicode_literal(w, mb);
    };
    "japanese"_test = []() {
        const auto w = L"おはようございます";
        const auto mb = "おはようございます";
        test_unicode_literal(w, mb);
    };
    "vietnamse"_test = []() {
        const auto w = L"Cộng hòa xã hội chủ nghĩa Việt Nam";
        const auto mb = "Cộng hòa xã hội chủ nghĩa Việt Nam";
        test_unicode_literal(w, mb);
    };
}

void test_replace(FILE*) {
    using namespace testing;

    "replace_space"_test = []() {
        const auto in = "이게 현실인 거에요"s;
        const auto out = wbcs_replace(in);
        cout << in << ' ' << in.length() << endl;
        cout << out << ' ' << out.length() << endl;
        testing::expect(in.length() == out.length());

        const auto exp = "이게\x20현실인\x20거에요"s;
        cout << exp << ' ' << exp.length() << endl;
        testing::expect(out == exp);
    };
    "replace_delims"_test = []() {
        const auto out = wbcs_replace("hello,world.11"s, "1,.", "!");
        testing::expect(out == "hello!world!!!"s);
    };
}

using file_owner_t = unique_ptr<FILE, int (*)(FILE*)>;
auto file_open(const gfs::path& fpath) -> file_owner_t;
auto file_append(const gfs::path& fpath) -> file_owner_t;

void get_asset_dir(gfs::path& dir);
void test_file(const gfs::path& asset_dir);
void test_stream(const gfs::path& asset_dir);

/**
 * @details argv[1] is required for execution
 * ```
 * ${program} ""        // native locale
 * ${program} "POSIX"   // POSIX (C) locale
 * ```
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fputs("argv[1] is required\n", stderr);
        return 1;
    }
    if (auto cat = setlocale(LC_ALL, argv[1]))
        fprintf(stdout, "setlocale(LC_ALL): '%s' -> '%s'\n", argv[1], cat);

    test_languages(stderr);
    test_replace(stderr);

    using namespace testing;

    "ghc_filesystem"_test = []() {
        gfs::path assets{};
        get_asset_dir(assets);
        {
            test_file(assets);
            test_stream(assets);
        }
    };
    return 0;
}

void get_asset_dir(gfs::path& dir) {
#if defined(UTF8_ASSET_DIR)
    dir = UTF8_ASSET_DIR;
#else
    dir = gfs::current_path();
#endif
}

auto file_open(const gfs::path& fpath) -> file_owner_t {
    FILE* fp{};
    if (auto ec = wbcs_open(&fp, fpath.generic_wstring()))
        throw system_error{static_cast<int>(ec), system_category()};
    return {fp, &fclose};
}

auto file_append(const gfs::path& fpath) -> file_owner_t {
    FILE* fp{};
    if (auto ec = wbcs_append(&fp, fpath.generic_wstring()))
        throw system_error{static_cast<int>(ec), system_category()};
    return {fp, &fclose};
}

/**
 * @todo https://golang.org/src/path/filepath/path_test.go
 */
void test_file(const gfs::path& asset_dir) {
    using namespace testing;

    "file_open_ENOENT"_test = [&asset_dir]() {
        std::string fname{};
        testing::eq(wbcs_w2mb(L"없는 파일", fname), 0);
        const auto fpath = asset_dir / fname;
        testing::expect(gfs::exists(fpath) == false);

        FILE* fp{};
        auto ec = wbcs_open(&fp, fpath.generic_wstring());
        testing::expect(fp == nullptr);
        testing::expect(ec == ENOENT);
    };
    "file_create"_test = [&asset_dir]() {
        std::string fname{};
        testing::eq(wbcs_w2mb(L"있는 파일", fname), 0);
        const auto fpath = asset_dir / fname;

        if (gfs::exists(fpath))
            gfs::remove(fpath);
        {
            FILE* fp{};
            testing::expect(wbcs_create(&fp, fpath.generic_wstring()) == 0);
            testing::expect(fclose(fp) == 0);
            testing::expect(fp != nullptr);
        }
        {
            testing::expect(gfs::exists(fpath));
            testing::expect(file_open(fpath) != nullptr);
        }
    };
    "file_append"_test = [&asset_dir]() {
        std::string fname{};
        testing::eq(wbcs_w2mb(L"있는 파일", fname), 0);
        const auto fpath = asset_dir / fname;

        testing::expect(gfs::exists(fpath));
        auto fp = file_append(fpath);
        testing::expect(fp != nullptr);

        const auto txt = fpath.generic_wstring();
        const auto flen = gfs::file_size(fpath);

        const auto wsz = fwprintf(fp.get(), txt.c_str());
        testing::eq(errno, 0); // no error for write
        testing::eq(txt.length(), wsz);
        testing::eq(gfs::file_size(fpath), flen + wsz); // appended?
    };
}

/**
 * @todo https://golang.org/src/path/filepath/path_test.go
 */
void test_stream(const gfs::path& asset_dir) {
    using namespace testing;
    const auto fpath = asset_dir / u8"있는 파일";

    "istream"_test = [&fpath]() {
        std::ifstream fin{fpath.generic_string()};
        testing::expect(fin.is_open());
        testing::eq(wbcs_empty_locale(fin), 0); // reset the locale

        std::string utf8{};
        std::getline(fin, utf8);
        testing::expect(utf8.empty() == false);
        cout << utf8 << endl;
    };
    "wistream"_test = [&fpath]() {
        std::wifstream fin{fpath.generic_string()};
        testing::expect(fin.is_open());
        testing::eq(wbcs_empty_locale(fin), 0); // reset the locale

        std::wstring wtxt{};
        std::getline(fin, wtxt);
        testing::expect(wtxt.empty() == false);
        wcout << wtxt << endl;
    };
}

#if __has_include(<boost/filesystem.hpp>)
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

namespace bsys = boost::system;
namespace bfs = boost::filesystem;

uint32_t get_asset_dir(bfs::path& dir) {
#if defined(UTF8_ASSET_DIR)
    dir = UTF8_ASSET_DIR;
    return 0;
#else
    bsys::error_code ec{};
    dir = bfs::current_path(ec);
    if (ec)
        return ec.value();
#endif
}

#endif // <boost/filesystem.hpp>
