#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <string>
#include <map>

std::string deescape_url(const char *url);

class Request
{
    public:
        Request();
        virtual void parse_url(const char* buf, int length);
        int Initialize(const char* buf, int length);
        static std::string deescape_url(const char *url);

        std::string  path_;
        std::string  query_;
        std::map<std::string,std::string>  query_map_;
        std::string  body_;
};

#endif //__REQUEST_HPP__
