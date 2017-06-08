SYSTEMC_HOME=/usr/cadtool/systemc-2.3
OCP_CHANNEL=/usr/cadtool/ocpchannels
OCP_SOCKET=/usr/cadtool/ocp_sockets_2_2x2_2


USE_SC_DYNAMIC_PROC=-DSC_INCLUDE_DYNAMIC_PROCESSES

OPTIMIZE_FLAG=-O0
DEBUG_FLAG=-g
CXXFLAGS = ${OPTIMIZE_FLAG} -Wall ${DEBUG_FLAG}
LDFLAGS = -Wl,-rpath=../DRAMSim2/DRAMSim2

LDLIBS = -lsystemc #-ldramsim

INCLUDE_DIRS = -I.
INCLUDE_DIRS += -I../
INCLUDE_DIRS += -I../DRAMSim2/DRAMSim2
INCLUDE_DIRS += -I"$(OCP_SOCKET)/include"
INCLUDE_DIRS += -I"$(SYSTEMC_HOME)/include/tlm_utils"
INCLUDE_DIRS += -I"$(SYSTEMC_HOME)/include"

LIB_DIRS = -L.
LIB_DIRS += -L../
LIB_DIRS += -L../DRAMSim2/DRAMSim2
LIB_DIRS += -L$(SYSTEMC_HOME)/lib-linux64

EXE = Test
SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $(LIB_DIRS) $(OBJ) -o $@ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(USE_SC_DYNAMIC_PROC) $(INCLUDE_DIRS) -c $< -o $@ $(COMPILER_ERROR)



#test: main.cpp OCPWrappedMemory.cpp TraceBasedOCPMaster.cpp
#	$(CXX) $(INCLUDE_DIRS) $(LIB_DIRS) -g -o test OCPWrappedMemory.cpp TraceBasedOCPMaster.cpp main.cpp -ldramsim -lsystemc -DSC_INCLUDE_DYNAMIC_PROCESSES -Wl,-rpath=../DRAMSim2

clean: 
	rm -f $(OBJ) $(EXE)

cleanlog:
	rm -f *.log	
