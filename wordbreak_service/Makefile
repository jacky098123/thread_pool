CC	        = g++

INC	        = -I. -I/usr/local/log4cplus/include -I../include 
INC         += -I/home/yangrq/github/kxutil4/include
INC         += -I/home/yangrq/cheetah/src/wordbreak/include

LDFLAGS	    = -g -DDEBUG -L/usr/local/log4cplus/lib -llog4cplus  -L../lib -lthread_pool 
LDFLAGS     += -L/home/yangrq/github/kxutil4/lib -lkxutil4
LDFLAGS     += -L/home/yangrq/cheetah/src/wordbreak/lib -lwordbreak
LDFLAGS     += -lpthread -ldl 

CPPFLAGS    = -Wall -DDEBUG -g $(INC)

TARGET	    = wordbreak_service

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

