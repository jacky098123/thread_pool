CC	        = g++
INC	        = -I. -I/usr/local/log4cplus/include -I/usr/local/thread_pool/include

LDFLAGS	    = -g -DDEBUG -L/usr/local/log4cplus/lib -llog4cplus  -L/usr/local/thread_pool/lib -lthread_pool
LDFLAGS     += -lpthread -ldl 

CPPFLAGS    = -Wall -DDEBUG -g $(INC)

TARGET	    = test_thread_pool

OBJ_DIR     =   .obj
MKDIRS      =   $(OBJ_DIR)



SVR_SRC	= $(wildcard *.cpp)
SVR_OBJ	= $(addprefix $(OBJ_DIR)/,$(subst .cpp,.o,$(SVR_SRC))) $(wildcard ../.obj/*.o)
SEC_OBJ = $(wildcard ../.obj/*.o)

OBJS	= $(SVR_OBJ)

all : $(TARGET)

$(TARGET) : $(OBJS) | $(MKDIRS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o : %.cpp | $(MKDIRS)
	$(CC) $(CPPFLAGS) -c $< -o $@

clean: 
	rm -rf $(OBJS) $(TARGET) *.so  $(OBJ_DIR) core 

$(sort $(MKDIRS)):
	@mkdir -p $@

