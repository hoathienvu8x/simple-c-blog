#include <iostream>
#include <string>
#include <stdlib.h>
#include "http.h"
#include "html.h"

int main(int argc, char **argv) {
    set_content_type("text/html; charset=utf-8");
    html_doctype();
    html_begin();
    head_begin();
    title_tag("CPP Blog");
    head_end();
    body_begin();
    h1_tag("This is CPP Blog");
    p_tag("This is description CPP Blog");
    blockquote_tag("This is simple and the first idea blog on c code, using cgi + sqlite to store database");
    body_end();
    html_end();
    return 0;
}
