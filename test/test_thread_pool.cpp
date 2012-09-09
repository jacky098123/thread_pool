#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pool_common.hpp"

int handle_yang(const Request &request, FunctionContext_t &context) {
	DECLARE_POOL_LOG;

	LOG4CPLUS_DEBUG(POOL_LOG, "handle_echo called.");

	strcpy(context.write_buffer, "handle_echo");
	context.write_current = strlen(context.write_buffer);

	return 200;
}

int handle_rong(const Request &request, FunctionContext_t &context) {
	DECLARE_POOL_LOG;

	LOG4CPLUS_DEBUG(POOL_LOG, "handle_math called.");

	strcpy(context.write_buffer, "handle_math");
	context.write_current = strlen(context.write_buffer);

	return 200;
}

int handle_quan(const Request &request, FunctionContext_t &context) {
	DECLARE_POOL_LOG;

	LOG4CPLUS_DEBUG(POOL_LOG, "handle_time called.");

	strcpy(context.write_buffer, "handle_time");
	context.write_current = strlen(context.write_buffer);

	return 200;
}

void business_hook() {
	register_function("yang", handle_yang);
	register_function("rong", handle_rong);
	register_function("quan", handle_quan);
}
