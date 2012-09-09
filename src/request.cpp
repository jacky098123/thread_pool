#include "request.hpp"

#include <stdlib.h>
#include <string.h>
#include "text.hpp"

using namespace std;

string deescape_url(const char *url)
{
    size_t i = 0;
    size_t k = 0;
    size_t n = strlen(url);
    char buf[n+1];

    while (i < n)
    {
        if (url[i] == '%')
        {
            buf[k++] = strtol(string(url+i+1, 2).c_str(),
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

    return string(buf);
}


Request::Request(const char* buf, int length) {
    parse_url(buf, length);
}

void Request::parse_url(const char* buf, int length) {
    const char *p = buf;
    while (*p != ' ' && (p-buf) < length) p++;
    while (*p == ' ' && (p-buf) < length) p++;
    while (*p == '/' && (p-buf) < length) p++;

    const char *p_2 = p;
    while ((p_2 - buf) < length && *p_2 != ' ') p_2++;

    string source = string(p, p_2 - p);
    string tmp = deescape_url(source.c_str());

    size_t pos = tmp.find("?");

    if (pos == string::npos) {
        path = tmp;
        query = "";
    } else {
        path = tmp.substr(0, pos);
        query = tmp.substr(pos+1);
        vector<string>  items;
        kxutil3::Text::Segment(query, "&", items);
        for (size_t i=0; i<items.size(); i++) {
            vector<string> kv;
            kxutil3::Text::Segment(items[i], "=", kv);
            if (kv.size() != 2) {
                continue;
            }
            query_map.insert(map<string,string>::value_type(kv[0], kv[1]));
        }
    }
}
