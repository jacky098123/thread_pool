#include "request.hpp"

#include <stdlib.h>
#include <string.h>
#include <vector>
#include "text/text.hpp"

#define HEADER_BODY_SEPERATOR "\r\n\r\n"

std::string Request::deescape_url(const char *url)
{
    size_t i = 0;
    size_t k = 0;
    size_t n = strlen(url);
    char buf[n+1];

    while (i < n)
    {
        if (url[i] == '%')
        {
            buf[k++] = strtol(std::string(url+i+1, 2).c_str(),
                              NULL,
                              16);
            i += 3;
        }
        else if (url[i] == '+')
        {
            buf[k++] = ' ';
            ++i;
        }
        else
        {
            buf[k++] = url[i++];
        }
    }
    buf[k] = 0;
    buf[n] = 0;

    return std::string(buf);
}


Request::Request() {
    this->path_ = "";
    this->query_ = "";
    this->query_map_.clear();
    this->body_ = "";
}

int Request::Initialize(const char* buf, int length) {
    this->path_ = "";
    this->query_ = "";
    this->query_map_.clear();
    this->body_ = "";

    parse_url(buf, length);
    return 0;
}

void Request::parse_url(const char* buf, int length) {
    const char *p = buf;
    while (*p != ' ' && (p-buf) < length) p++;
    while (*p == ' ' && (p-buf) < length) p++;
    while (*p == '/' && (p-buf) < length) p++;

    if (p - buf == length) {
        return;
    }

    const char *p_2 = p;
    while ((p_2 - buf) < length && *p_2 != ' ') p_2++;

    std::string source = std::string(p, p_2 - p);

    size_t pos = source.find("?");

    if (pos == std::string::npos) {
        this->path_ = source;
        this->query_ = "";
    } else {
        this->path_ = source.substr(0, pos);
        this->query_ = source.substr(pos+1);
        std::vector<std::string>  items;
        kxutil4::Text::Segment(this->query_, "&", items);
        for (size_t i=0; i<items.size(); i++) {
            vector<std::string> kv;
            kxutil4::Text::Segment(items[i], "=", kv);
            if (kv.size() != 2) {
                continue;
            }
            std::string tmp = Request::deescape_url(kv[1].c_str());
            this->query_map_.insert(map<std::string,std::string>::value_type(kv[0], tmp));
        }
    }

    const char* pbody = strstr(p_2, HEADER_BODY_SEPERATOR);
    if (pbody == NULL) {
        return;
    }

    this->body_ = string(pbody + 4, length - (pbody-buf) - 4);
}
