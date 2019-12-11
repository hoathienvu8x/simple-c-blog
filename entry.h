#ifndef _ENTRY_H
#define _ENTRY_H

#include <string>
#include "util.h"

typedef struct {
    std::string title, excerpt, content;
    term_t []category, []tag;
} entry_t;

std::string the_entry_url(term_t *t, std::string domain) {
    domain = rtrim(domain, "/");
    return domain + t->slug + "/";
}

#endif