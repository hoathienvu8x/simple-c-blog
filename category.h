#ifndef _CATEGORY_H
#define _CATEGORY_H

#include <string>
#include "util.h"

std::string the_term_url(term_t *t, std::string domain) {
    domain = rtrim(domain, "/");
    return domain + t->slug + "/";
}

#endif