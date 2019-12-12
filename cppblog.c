#include <iostream>
#include <string>
#include <stdlib.h>
#include <regex>
#include "util.h"
#include "http.h"
#include "html.h"
#include "sqlite3.h"
#include "dump.h"

std::string current_path = "./";
std::string dbFile = "cppblog.db";
sqlite3 *db;

std::regex make_regex(std::string re, bool ignorecase = false) {
    if (ignorecase) {
        return std::regex(re, std::regex_constants::ECMAScript | std::regex_constants::icase);
    }
    return std::regex(re);
}

int main(int argc, char **argv) {
    current_path = getexepath();
    if ( ! is_dir( current_path + "datas" ) && ! mkdirAll( current_path + "datas" )) {
        std::cout << "Could not create sqlite quote store data" << std::endl;
		return 1;
    }

    int rc;
    bool dbinstall = false;
    dbFile = current_path + "datas" + PATH_SEPARATOR + dbFile;
    if ( ! file_exists( dbFile ) ) {
        dbinstall = true;
    }
    rc = sqlite3_open(dbFile.c_str(), &db);
    if( rc != SQLITE_OK ){
        std::cout << "DB Error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }
    if (dbinstall) {
        char *zErrMsg = 0;
        rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS posts ( id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, slug TEXT NOT NULL, excerpt TEXT NOT NULL, content TEXT NOT NULL, pubdate TEXT NOT NULL, tags TEXT NOT NULL );", NULL, 0, &zErrMsg);
        if ( rc != SQLITE_OK ) {
            std::cout << "SQL error create posts table: " << zErrMsg << std::endl;
            sqlite3_close(db);
            return 1;
        }
        rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS terms ( id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, slug TEXT NOT NULL );", NULL, 0, &zErrMsg);
        if ( rc != SQLITE_OK ) {
            std::cout << "SQL error create terms table: " << zErrMsg << std::endl;
            sqlite3_close(db);
            return 1;
        }
        rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS post_terms ( id INTEGER PRIMARY KEY AUTOINCREMENT, post_id INTEGER NOT NULL DEFAULT 0, term_id INTEGER NOT NULL DEFAULT 0 );", NULL, 0, &zErrMsg);
        if ( rc != SQLITE_OK ) {
            std::cout << "SQL error create post terms table: " << zErrMsg << std::endl;
            sqlite3_close(db);
            return 1;
        }
    }
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
