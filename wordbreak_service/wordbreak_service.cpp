#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <map>

#include "pool_common.hpp"
#include "init.h" //wordbreak
#include "encoding/utf8_conv.hpp"

#define HEAD_SEPERATOR "\r\n\r\n"

int handle_wordbreak(const Request &request, FunctionContext_t &context) {
	DECLARE_POOL_LOG;

	LOG4CPLUS_DEBUG(POOL_LOG, "handle_wordbreak called.");

    std::map<string,string>::const_iterator it = request.query_map.find("word");
    if (it == request.query_map.end()) {
        // no parameter word
        string ret_content = "{\"ret_code\":-1 ,\"reason\": \"no parameter word\"}";
        memcpy(context.write_buffer, ret_content.c_str(), ret_content.size());
        context.write_current = ret_content.size();
        return 200;
    }

    string utf8_str = it->second;
	LOG4CPLUS_DEBUG(POOL_LOG, "utf8_str: " << utf8_str);
    string gbk_content = kxutil3::Encoding::UTF82GBK(utf8_str);
    string gbk_keyword;

    WordBreak(gbk_content, gbk_keyword);

    string utf8_keyword = kxutil3::Encoding::GBK2UTF8(gbk_keyword);
	LOG4CPLUS_DEBUG(POOL_LOG, "utf8_keyword: " << utf8_keyword);

    string ret_content = "{\"ret_code\":0, \"keyword\": \"" + utf8_keyword + "\"}";

	memcpy(context.write_buffer, ret_content.c_str(), ret_content.size());
	context.write_current = ret_content.size();

	return 200;
}

void business_hook() {
    string wordbreak_path;
    Config::Instance()->GetConfStr("general", "wordbreak_path", wordbreak_path);
    if (wordbreak_path.size() < 10) {
        printf("wordbreak_path err: %s\n", wordbreak_path.c_str());
        exit(1);
    }

    cut_load((char*)wordbreak_path.c_str());
	register_function("wordbreak", handle_wordbreak);
}
