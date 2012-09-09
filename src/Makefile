ifndef INSTALL_DIR
INSTALL_DIR   = /usr/local/thread_pool
endif

CC	        = g++

INC         = -I/usr/local/log4cplus/include

LDFLAGS	    = -L/usr/local/log4cplus/lib -llog4cplus
LDFLAGS     += -lpthread

CPPFLAGS    = -Wall -DDEBUG -g $(INC)

OBJ_DIR     =   .obj

LIBCPPFILES = commu_socket.cpp  config.cpp  thread_pool.cpp  request.cpp  text.cpp
TESTCPPFILES= $(LIBCPPFILES) business_server.cpp
LIBOBJFILES = $(LIBCPPFILES:%.cpp=$(OBJ_DIR)/%.o)
TESTOBJFILES= $(TESTCPPFILES:%.cpp=$(OBJ_DIR)/%.o)


MKDIRS      =   $(OBJ_DIR)

LIB_POOL    = libthread_pool.a
TEST_POOL   = test_pool

lib : $(LIB_POOL)
test : $(TEST_POOL)

test_pool : $(TESTOBJFILES) | $(MKDIRS)
	$(CC) $(TESTOBJFILES) $(LDFLAGS) -o $@

libthread_pool.a : $(LIBOBJFILES) | $(MKDIRS)
	ar rcv $@ $(LIBOBJFILES)

$(OBJ_DIR)/%.o : %.cpp | $(MKDIRS)
	$(CC) $(CPPFLAGS) -c $< -o $@

install:
	install -d $(INSTALL_DIR)/include
	install -d $(INSTALL_DIR)/lib
	install $(LIB_POOL) $(INSTALL_DIR)/lib
	install -m 666 pool_common.hpp $(INSTALL_DIR)/include/
	install -m 666 request.hpp $(INSTALL_DIR)/include/


clean: 
	rm -rf $(LIBOBJFILES) $(TESTOBJFILES) *.so  $(OBJ_DIR) core 

$(sort $(MKDIRS)):
	@mkdir -p $@ 
