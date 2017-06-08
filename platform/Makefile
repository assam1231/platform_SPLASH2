SHELL = /bin/sh

include ../Makefile.common

ifeq ($(D),1)
	CFLAGS += -DEN_ROUTER_DEBUG
endif

#Define the directories containing header files
INCLUDES = -I$(SYSTEMC_HOME)/include \
           -I$(SYSTEMC_HOME)/include/tlm_utils \
           -I$(OCP_HOME)/include \
           -I$(NOC_DIR) \
           -I$(NOC_DIR)/NTS \
           -I$(NOC_DIR)/router \
           -I$(DRAMSim2) \
           -I$(MEMSYS) \
           -I$(TRAFFIC_MODULE_DIR) \
           -I$(XML_PARSER_DIR) \
		   -I$(NI_DIR) \
		   -I$(SIMPLE_NI_DIR) \
		   -I.. \
		   -I$(ADDRESS_MAP) \
		   -I$(ROUTER_DIR)

#Define the library directories
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LFLAGS = -L$(SYSTEMC_HOME)/lib-linux64
endif
ifeq ($(UNAME_S),Darwin)
	LFLAGS = -L$(SYSTEMC_HOME)/lib-macosx64
endif

#Define the library to link
LIBS = -lsystemc

#Define the source files
SRCS = main.cpp 
SRCS += $(wildcard $(TRAFFIC_MODULE_DIR)/*.cpp)
SRCS += $(wildcard $(XML_PARSER_DIR)/*.cpp)
SRCS += $(wildcard $(SIMPLE_NI_DIR)/*.cpp)

SRCS += $(wildcard $(NOC_DIR)/*.cpp)
SRCS += $(wildcard $(NOC_DIR)/NTS/*.cpp)
SRCS += $(wildcard $(MEMSYS)/*.cpp)
SRCS += $(wildcard $(DRAMSim2)/*.cpp)
SRCS += $(wildcard $(NI_DIR)/*.cpp)

#Define object files
OBJS = $(SRCS:.cpp=.o)

DEPS = $(OBJS:.o=.d)
      
MAIN= simple_mesh_2d

all: $(MAIN)

$(MAIN): $(OBJS)
	@echo linking
	$(V) $(CC) $(LFLAGS) $(LIBS) -o $@ $^

# pull in dependency info for *existing* .o files
-include $(DEPS)

%.o: %.cpp
	@echo compiling $< 
	$(V) $(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
	$(V) $(CC) -MM $(CFLAGS) $(INCLUDES) $*.cpp > $*.d


clean: 
	@echo removeing executable, object and dependence files.
	@rm -f $(OBJS) $(MAIN) $(OBJS:.o=.d)

clean_log:
	@echo removing log files.
	@rm -f *.log
	@rm -rf log/
