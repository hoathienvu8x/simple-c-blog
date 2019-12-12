#ifndef _UTIL_H
#define _UTIL_H

#include <assert.h>
#include <string>
#include <iostream>
#include <sys/stat.h>
#if defined _WIN32 || defined __CYGWIN__ || defined WIN32
    #include <direct.h>
    #define GetCurrentDir _getcwd
    #define PATH_SEPARATOR "\\" 
    #define stat _stat
    #define mkdir(p,m) _mkdir(p)
    #include <windows.h>
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
    #define PATH_SEPARATOR "/" // https://stackoverflow.com/a/12971978
#endif
typedef struct {
    std::string name, slug;
} term_t;
// https://stackoverflow.com/a/12774387
bool file_exists(const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
bool is_dir(const std::string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}
bool mkdirAll(const std::string& path) {
    if (is_dir(path)) {
        return true;
    }
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
    if (ret == 0) {
        return true;
    }
    switch (errno) {
        case ENOENT: {
            size_t pos = path.find_last_of(PATH_SEPARATOR);
            if (pos == std::string::npos) {
                return false;
            }
            if ( ! mkdirAll(path.substr(0, pos)) ) {
                return false;
            }
        }
        return 0 == mkdir(path.c_str(), mode);
        case EEXIST: return is_dir(path);
        default: return false;
    }
}
std::string getexepath() {
    char buff[FILENAME_MAX];
    if(GetCurrentDir( buff, FILENAME_MAX ) == NULL) {
        return "./";
    }
    std::string current_working_dir(buff);
    return current_working_dir + PATH_SEPARATOR;
}
std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
    str.erase(0, str.find_first_not_of(chars));
    return str;
}
std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}
std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
    return ltrim(rtrim(str, chars), chars);
}
// https://github.com/yhirose/cpp-httplib/blob/master/httplib.h#L921
bool is_hex(char c, int &v) {
    if (0x20 <= c && isdigit(c)) {
        v = c - '0';
        return true;
    } else if ('A' <= c && c <= 'F') {
        v = c - 'A' + 10;
        return true;
    } else if ('a' <= c && c <= 'f') {
        v = c - 'a' + 10;
        return true;
    }
    return false;
}
// https://github.com/yhirose/cpp-httplib/blob/master/httplib.h#L935
bool from_hex_to_i(const std::string &s, size_t i, size_t cnt, int &val) {
    if (i >= s.size()) {
        return false;
    }
    val = 0;
    for (; cnt; i++, cnt--) {
        if (!s[i]) {
            return false;
        }
        int v = 0;
        if (is_hex(s[i], v)) {
            val = val * 16 + v;
        } else {
            return false;
        }
    }
    return true;
}
// https://github.com/yhirose/cpp-httplib/blob/master/httplib.h#L962
size_t to_utf8(int code, char *buff) {
    if (code < 0x0080) {
        buff[0] = (code & 0x7F);
        return 1;
    } else if (code < 0x0800) {
        buff[0] = (0xC0 | ((code >> 6) & 0x1F));
        buff[1] = (0x80 | (code & 0x3F));
        return 2;
    } else if (code < 0xD800) {
        buff[0] = (0xE0 | ((code >> 12) & 0xF));
        buff[1] = (0x80 | ((code >> 6) & 0x3F));
        buff[2] = (0x80 | (code & 0x3F));
        return 3;
    } else if (code < 0xE000) { // D800 - DFFF is invalid...
        return 0;
    } else if (code < 0x10000) {
        buff[0] = (0xE0 | ((code >> 12) & 0xF));
        buff[1] = (0x80 | ((code >> 6) & 0x3F));
        buff[2] = (0x80 | (code & 0x3F));
        return 3;
    } else if (code < 0x110000) {
        buff[0] = (0xF0 | ((code >> 18) & 0x7));
        buff[1] = (0x80 | ((code >> 12) & 0x3F));
        buff[2] = (0x80 | ((code >> 6) & 0x3F));
        buff[3] = (0x80 | (code & 0x3F));
        return 4;
    }
    // NOTREACHED
    return 0;
}
// https://github.com/yhirose/cpp-httplib/blob/master/httplib.h#L1844
std::string encode_url(const std::string &s) {
    std::string result;
    for (auto i = 0; s[i]; i++) {
        switch (s[i]) {
            case ' ': result += "%20"; break;
            case '+': result += "%2B"; break;
            case '\r': result += "%0D"; break;
            case '\n': result += "%0A"; break;
            case '\'': result += "%27"; break;
            case ',': result += "%2C"; break;
            case ':': result += "%3A"; break;
            case ';': result += "%3B"; break;
            default:
                auto c = static_cast<uint8_t>(s[i]);
                if (c >= 0x80) {
                    result += '%';
                    char hex[4];
                    size_t len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
                    assert(len == 2);
                    result.append(hex, len);
                } else {
                    result += s[i];
                }
                break;
        }
    }
    return result;
}
// https://github.com/yhirose/cpp-httplib/blob/master/httplib.h#L1875
std::string decode_url(const std::string &s) {
    std::string result;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 1 < s.size()) {
            if (s[i + 1] == 'u') {
                int val = 0;
                if (from_hex_to_i(s, i + 2, 4, val)) {
                    // 4 digits Unicode codes
                    char buff[4];
                    size_t len = to_utf8(val, buff);
                    if (len > 0) {
                        result.append(buff, len);
                    }
                    i += 5; // 'u0000'
                } else {
                    result += s[i];
                }
            } else {
                int val = 0;
                if (from_hex_to_i(s, i + 1, 2, val)) {
                    // 2 digits hex codes
                    result += static_cast<char>(val);
                    i += 2; // '00'
                } else {
                    result += s[i];
                }
            }
        } else if (s[i] == '+') {
            result += ' ';
        } else {
            result += s[i];
        }
    }
    return result;
}
#endif