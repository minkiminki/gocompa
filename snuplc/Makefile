CC=g++
#CCFLAGS=-std=c++0x -Iinclude -g -O0
CCFLAGS=-std=c++0x -Iinclude -O2

INC_DIR=include
SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin
PRIV_DIR=private

DEPS=ir.h \
		ir2.h \
		opt_pre.h \
		opt_rpm.h \
		opt_bba.h \
		opt_tco.h \
		opt_ue.h \
		opt_ra.h \
		opt_cpg.h \
		opt_inl.h \
		opt_ssa.h \
		opt_la.h \
		opt.h \
		backend.h
SRC=ir.cpp \
		ir2.cpp \
		liveness.cpp \
		opt_pre.cpp \
		opt_rpm.cpp \
		opt_bba.cpp \
		opt_tco.cpp \
		opt_cpg.cpp \
		opt_inl.cpp \
		opt_ue.cpp \
		opt_ra.cpp \
		opt_ssa.cpp \
		opt_la.cpp \
		opt.cpp \
		symbolinfo.cpp \
		backend_default.cpp \
		backend.cpp \
		backend64.cpp
MOD=scanner.cpp \
		parser.cpp \
		type.cpp \
		symtab.cpp \
		data.cpp \
		ast.cpp

DEPS_=$(patsubst %,$(INC_DIR)/%,$(DEPS))
OBJ=$(patsubst %.cpp,$(OBJ_DIR)/%.o, $(SRC))
MOD_DEPS=$(patsubst %.cpp,$(INC_DIR)/%.h,$(MOD))
BIN=$(patsubst %.cpp,$(BIN_DIR)/%.o,$(MOD))

.PHONY: clean doc bin

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS_)
	$(CC) $(CCFLAGS) -c -o $@ $<

$(BIN_DIR)/%.o: $(PRIV_DIR)/%.cpp $(MOD_DEPS)
	$(CC) $(CCFLAGS) -c -o $@ $<

all: snuplc

bin: ${BIN}

snuplc: $(OBJ_DIR)/snuplc.o $(OBJ) $(BIN)
	$(CC) $(CCFLAGS) -o $@ $(OBJ_DIR)/snuplc.o $(OBJ) $(BIN)

doc:
	doxygen

clean:
	rm -rf $(OBJ_DIR)/*.o snuplc

mrproper: clean
	rm -rf doc/*
	rm -rf ${BIN_DIR}/*
