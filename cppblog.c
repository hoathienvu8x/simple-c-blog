#include <iostream>
#include <string>
#include <stdlib.h>
#include <regex>
#include "util.h"
#include "http.h"
#include "html.h"

std::regex make_regex(std::string re, bool ignorecase = false) {
    if (ignorecase) {
        return std::regex(re, std::regex_constants::ECMAScript | std::regex_constants::icase);
    }
    return std::regex(re);
}

int main(int argc, char **argv) {
    set_content_type("text/html; charset=utf-8");
    html_doctype();
    html_begin();
    head_begin();
    meta_equiv();
    meta_viewport();
    title_tag("CPP Blog");
    site_stylesheet("/");
    head_end();
    body_begin();
    h1_tag("This is CPP Blog");
    p_tag("This is description CPP Blog");
    blockquote_tag("This is simple and the first idea blog on c code, using cgi + sqlite to store database");
    const char *path_info = getenv("REQUEST_URI");
    if (path_info != NULL) {
		if (strcmp(path_info,"/") == 0) {
			p_tag("That homepage");
			goto BREAKOUT;
			return 0;
		}
        p_tag("Request URI: " + std::string(path_info));
        std::regex rx = make_regex("^/tu-khoa/(.+)?/?$", true);
        std::smatch res;
        std::string path = std::string(path_info); // https://stackoverflow.com/a/30495370
        if(std::regex_search(path, res, rx)) {
            if (res.size() > 1) {
                p_tag("Tag: " + decode_url(std::string(res[1])));
                goto BREAKOUT;
				return 0;
            }
        }
        rx = make_regex("^/chuyen-muc/(.+)?/?$", true);
        if(std::regex_search(path, res, rx)) {
            if (res.size() > 1) {
                p_tag("Category: " + decode_url(std::string(res[1])));
                goto BREAKOUT;
				return 0;
            }
        }
        rx = make_regex("^/(.+)/amp?/?$", true);
        if(std::regex_search(path, res, rx)) {
            if (res.size() > 1) {
                p_tag("Entry AMP: " + decode_url(std::string(res[1])));
                goto BREAKOUT;
				return 0;
            }
        }
        rx = make_regex("^/(.+)?/?$", true);
        if(std::regex_search(path, res, rx)) {
            if (res.size() > 1) {
                p_tag("Entry: " + decode_url(std::string(res[1])));
                goto BREAKOUT;
				return 0;
            }
        }
    }
BREAKOUT:
    body_end();
    html_end();
    return 0;
}
