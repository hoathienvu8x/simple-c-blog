#ifndef _HTTP_H
#define _HTTP_H

#include <iostream>
#include <string>

void set_content_type(std::string content_type) {
    std::cout << "Content-Type: " << content_type << "\r\n\r\n";
}

#endif
