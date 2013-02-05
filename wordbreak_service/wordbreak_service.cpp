#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include <map>

#include "pool_common.hpp"  // thread_pool

#include "init.h"           //wordbreak
#include "sys/config.hpp"   // kxutil4
#include "text/text.hpp"    // kxutil4
#include "encoding/utf8_conv.hpp"   // kxutil4

#define HEAD_SEPERATOR "\r\n\r\n"


int handle_wordbreak(const Request &request, FunctionContext_t &context) {
	DECLARE_POOL_LOG;

	LOG4CPLUS_DEBUG(POOL_LOG, "handle_wordbreak called.");

    if (request.body_.size() == 0) {
        std::string ret_content = "{\"ret_code\":-1 ,\"reason\": \"body is empty\"}";
        memcpy(context.write_buffer, ret_content.c_str(), ret_content.size());
        context.write_current = ret_content.size();
	    LOG4CPLUS_WARN(POOL_LOG, "body is empty");
        return 200;
    }

    std::map<std::string,std::string> para  = kxutil4::Text::String2Map(request.body_, "&", "=");
    std::map<std::string,std::string>::const_iterator it = para.find("word");
    if (it == para.end()) {
        // no parameter word
        std::string ret_content = "{\"ret_code\":-1 ,\"reason\": \"body no word parameter\"}";
        memcpy(context.write_buffer, ret_content.c_str(), ret_content.size());
        context.write_current = ret_content.size();
	    LOG4CPLUS_WARN(POOL_LOG, "no word parameter: " << request.body_);
        return 200;
    }

    std::string utf8_str = Request::deescape_url(it->second.c_str());
	LOG4CPLUS_DEBUG(POOL_LOG, "utf8_str: " << utf8_str << ", size: " << utf8_str.size());
    std::string gbk_content = kxutil4::Encoding::UTF82GBK(utf8_str);
    std::string gbk_keyword;

    WordBreak(gbk_content, gbk_keyword);

    std::string utf8_keyword = kxutil4::Encoding::GBK2UTF8(gbk_keyword);
	LOG4CPLUS_DEBUG(POOL_LOG, "utf8_keyword: " << utf8_keyword);

    std::string ret_content = "{\"ret_code\":0, \"keyword\": \"" + utf8_keyword + "\"}";

	memcpy(context.write_buffer, ret_content.c_str(), ret_content.size());
	context.write_current = ret_content.size();

	return 200;
}

void business_hook() {
    std::string wordbreak_path;
    kxutil4::Config::Instance()->GetStr("general", "wordbreak_path", wordbreak_path);
    if (wordbreak_path.size() < 10) {
        printf("wordbreak_path err: %s\n", wordbreak_path.c_str());
        exit(1);
    }

    cut_load((char*)wordbreak_path.c_str());
	register_function("wordbreak", handle_wordbreak);
}
