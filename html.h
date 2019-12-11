#ifndef _HTML_H
#define _HTML_H

#include <iostream>
#include <string>

void html_doctype() {
    std::cout << "<!doctype html>";
}

void html_begin() {
    std::cout << "<html lang=\"en\">";
}

void html_end() {
    std::cout << "</html>";
}

void head_begin() {
    std::cout << "<head>";
}
void head_end() {
    std::cout << "</head>";
}

void body_begin() {
    std::cout << "<body>";
}
void body_end() {
    std::cout << "</body>";
}
void title_tag(std::string text) {
    std::cout << "<title>" << text << "</title>";
}

void h1_tag(std::string text) {
    std::cout << "<h1>" << text << "</h1>";
}

void h2_tag(std::string text) {
    std::cout << "<h2>" << text << "</h2>";
}

void h3_tag(std::string text) {
    std::cout << "<h3>" << text << "</h3>";
}

void h4_tag(std::string text) {
    std::cout << "<h4>" << text << "</h4>";
}

void h5_tag(std::string text) {
    std::cout << "<h5>" << text << "</h5>";
}

void h6_tag(std::string text) {
    std::cout << "<h6>" << text << "</h6>";
}

void p_tag(std::string text) {
    std::cout << "<p>" << text << "</p>";
}

void br_tag() {
    std::cout << "<br />";
}

void blockquote_tag(std::string text) {
    std::cout << "<blockquote>" << text << "</blockquote>";
}

void hr_tag() {
    std::cout << "<hr />";
}
#endif
