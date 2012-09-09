/*
 * client.cpp
 *
 *  Created on: Jun 3, 2012
 *      Author: yangrq
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pool_common.hpp"

int handle_echo(const Request &request, FunctionContext_t &context) {
	DECLARE_POOL_LOG;

	LOG4CPLUS_DEBUG(POOL_LOG, "handle_echo called.");

	strcpy(context.write_buffer, "handle_echo");
	context.write_current = strlen(context.write_buffer);

	return 200;
}

int handle_math(const Request &request, FunctionContext_t &context) {
	DECLARE_POOL_LOG;

	LOG4CPLUS_DEBUG(POOL_LOG, "handle_math called.");

	strcpy(context.write_buffer, "handle_math");
	context.write_current = strlen(context.write_buffer);

	return 200;
}

int handle_time(const Request &request, FunctionContext_t &context) {
	DECLARE_POOL_LOG;

	LOG4CPLUS_DEBUG(POOL_LOG, "handle_time called.");

	strcpy(context.write_buffer, "handle_time");
	context.write_current = strlen(context.write_buffer);

	return 200;
}

void business_hook() {
	register_function("echo", handle_echo);
	register_function("math", handle_math);
	register_function("time", handle_time);
}
