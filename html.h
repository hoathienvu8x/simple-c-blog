#ifndef _HTML_H
#define _HTML_H

#include <iostream>
#include <string>
#include <map>
#include "util.h"

typedef std::map<std::string, std::string> attribute_t;
// http://forums.codeguru.com/showthread.php?448809-C-Replacing-HTML-Character-Entities&p=1698435#post1698435
std::string htmlspecialchars(std::string str) {
    std::string subs[] = {"\"","'","&","<",">"};
    std::string reps[] = {"&quot;","&apos;","&amp;","&lt;","&gt;"};
    size_t found;
    for(int i = 0; i < 15; i++) {
        do {
            found = str.find(subs[i]);
            if (found != std::string::npos) {
                str.replace (found,subs[i].length(),reps[i]);
            }
        } while (found != std::string::npos);
    }
    return str;
}
// http://www.cplusplus.com/reference/map/map/insert/
std::string html_attributes(attribute_t *attrs = NULL) {
    std::stringstream stream;
    if (attrs != NULL) {
        for (auto attr = attrs->begin() ; attr != attrs->end(); ++attr) {
            stream << " " << attr->first << "=\"" << htmlspecialchars(attr->second) << "\"";
        }
    }
    return stream.str();
}

void html_doctype(attribute_t *attrs = NULL) {
    std::cout << "<!doctype html" << html_attributes(attrs) << ">";
}

void html_begin(attribute_t *attrs = NULL) {
    if (attrs == NULL) {
        attrs = new attribute_t();
        attrs->insert(std::pair<std::string, std::string>("lang","en"));
    }
    std::cout << "<html" << html_attributes(attrs) << ">";
}

void html_end() {
    std::cout << "</html>";
}

void head_begin(attribute_t *attrs = NULL) {
    std::cout << "<head" << html_attributes(attrs) << ">";
}
void meta_equiv(){
	std::cout << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />";
}
void meta_viewport() {
	std::cout << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0\" />";
}
void site_stylesheet(std::string domain = "/") {
	std::cout << "<link href=\"" << rtrim(domain,"/") << "/style.css\" rel=\"stylesheet\" type=\"text/css\" />";
}
void head_end() {
    std::cout << "</head>";
}

void body_begin(attribute_t *attrs = NULL) {
    std::cout << "<body" << html_attributes(attrs) << ">";
}
void body_end() {
    std::cout << "</body>";
}
void title_tag(std::string text) {
    std::cout << "<title>" << text << "</title>";
}

void h1_tag(std::string text, attribute_t *attrs = NULL) {
    std::cout << "<h1" << html_attributes(attrs) << ">" << text << "</h1>";
}

void h2_tag(std::string text, attribute_t *attrs = NULL) {
    std::cout << "<h2" << html_attributes(attrs) << ">" << text << "</h2>";
}

void h3_tag(std::string text, attribute_t *attrs = NULL) {
    std::cout << "<h3" << html_attributes(attrs) << ">" << text << "</h3>";
}

void h4_tag(std::string text, attribute_t *attrs = NULL) {
    std::cout << "<h4" << html_attributes(attrs) << ">" << text << "</h4>";
}

void h5_tag(std::string text, attribute_t *attrs = NULL) {
    std::cout << "<h5" << html_attributes(attrs) << ">" << text << "</h5>";
}

void h6_tag(std::string text, attribute_t *attrs = NULL) {
    std::cout << "<h6" << html_attributes(attrs) << ">" << text << "</h6>";
}

void p_tag(std::string text, attribute_t *attrs = NULL) {
    std::cout << "<p" << html_attributes(attrs) << ">" << text << "</p>";
}

void br_tag(attribute_t *attrs = NULL) {
    std::cout << "<br" << html_attributes(attrs) << " />";
}

void blockquote_tag(std::string text, attribute_t *attrs = NULL) {
    std::cout << "<blockquote" << html_attributes(attrs) << ">" << text << "</blockquote>";
}

void hr_tag(attribute_t *attrs = NULL) {
    std::cout << "<hr" << html_attributes(attrs) << " />";
}
#endif
