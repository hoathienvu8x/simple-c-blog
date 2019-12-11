#ifndef _TAG_H
#define _TAG_H

#include <string>
#include "util.h"

std::string the_tag_url(term_t *t, std::string domain) {
    domain = rtrim(domain, "/");
    return domain + t->slug + "/";
}

#endif