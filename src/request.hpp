#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <string>
#include <map>

using namespace std;

string deescape_url(const char *url);

class Request
{
    public:
        Request(const char* buf, int length);
        virtual void parse_url(const char* buf, int length);

        string  path;
        string  query;
        map<string,string>  query_map;
};

#endif //__REQUEST_HPP__
