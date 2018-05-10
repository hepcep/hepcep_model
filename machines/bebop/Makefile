model_src :=
test_src :=

CXX = mpicxx
CXXLD = mpicxx

SWIG = /usr/bin/swig
SED = sed

CXX_RELEASE_FLAGS = -Wall -O2 -g0 -std=c++11 -MMD -MP
CXX_DEBUG_FLAGS = -Wall -O0 -g3 -std=c++11 -MMD -MP

CXX_FLAGS = $(CXX_RELEASE_FLAGS) -fopenmp

INCLUDES :=
LIBS :=

SFW=/lcrc/project/MRSA/bebop/sfw

REPAST_LIB_DIR = $(SFW)/repast_hpc-2.2.0/lib
REPAST_LIB = repast_hpc-2.2.0
REPAST_INCLUDE = $(SFW)/repast_hpc-2.2.0/include

CHISIM_VERSION = 0.2
CHISIM_LIB_DIR = $(SFW)/chiSIM-$(CHISIM_VERSION)/lib
CHISIM_LIB = chiSIM-$(CHISIM_VERSION)
CHISIM_INCLUDE = $(SFW)/chiSIM-$(CHISIM_VERSION)/include

BOOST=/lcrc/project/MRSA/bebop/sfw/Boost/Boost_1.61
BOOST_INCLUDE = $(BOOST)/include
BOOST_LIB_DIR = $(BOOST)/lib
BOOST_LIBS = -lboost_mpi-mt -lboost_system-mt -lboost_filesystem-mt

# these are not necessary unless you are creating a tcl wrapper
# to use with swift/T
TCL_INCLUDE =  /blues/gpfs/home/software/spack-0.10.1/opt/spack/linux-centos7-x86_64/gcc-7.1.0/tcl-8.6.6-cq5hwyt3bhe3j3kxrem64xjiy4njoa2k/include
TCL_LIB     = -L/blues/gpfs/home/software/spack-0.10.1/opt/spack/linux-centos7-x86_64/gcc-7.1.0/tcl-8.6.6-cq5hwyt3bhe3j3kxrem64xjiy4njoa2k/lib -ltcl8.6
TCLSH       = /blues/gpfs/home/software/spack-0.10.1/opt/spack/linux-centos7-x86_64/gcc-7.1.0/tcl-8.6.6-cq5hwyt3bhe3j3kxrem64xjiy4njoa2k/bin/tclsh8.6
TCLSH_LOCAL = /blues/gpfs/home/software/spack-0.10.1/opt/spack/linux-centos7-x86_64/gcc-7.1.0/tcl-8.6.6-cq5hwyt3bhe3j3kxrem64xjiy4njoa2k/bin/tclsh8.6

INCLUDES += -I $(REPAST_INCLUDE)
INCLUDES += -I $(BOOST_INCLUDE)
INCLUDES += -I ../src -I $(CHISIM_INCLUDE)
INCLUDES += -I $(TCL_INCLUDE)

GTEST_HOME = $(HOME)/sfw/googletest
GTEST_LIB = $(GTEST_HOME)/lib/libgtest.a
TEST_INCLUDES = -I $(GTEST_HOME)/include

LIBS += -L $(REPAST_LIB_DIR) -l$(REPAST_LIB)
LIBS += -L $(BOOST_LIB_DIR) $(BOOST_LIBS)
LIBS += -L $(CHISIM_LIB_DIR) -l$(CHISIM_LIB)

LDFLAGS = -fopenmp
LIB_LDFLAGS = $(LDFLAGS)
LIB_EXT =

OS := $(shell uname)

ifeq ($(OS),Darwin)
 LIB_LDFLAGS += -dynamiclib
 LIB_EXT = dylib
else
 # linux
 LIB_CXX_FLAGS += -fPIC
 LIB_LDFLAGS += -shared
 LIB_EXT = so
endif

RPATHS += -Wl,-rpath -Wl,$(REPAST_LIB_DIR)
RPATHS += -Wl,-rpath -Wl,$(CHISIM_LIB_DIR)
RPATHS += -Wl,-rpath -Wl,$(BOOST_LIB_DIR)

SRC_DIR=../src
BUILD_DIR = ./build

include ../src/module.mk
include ../test/module.mk

# objects used by both executable and tests
OBJECTS :=
OBJECTS += $(subst .cpp,.o, $(addprefix $(BUILD_DIR)/, $(model_src)))

# objects and deps for executable
EXEC_OBJECTS := $(OBJECTS)
EXEC_OBJECTS += $(BUILD_DIR)/main.o

LIB_OBJECTS := $(OBJECTS)
LIB_OBJECTS += $(BUILD_DIR)/run.o

TEST_OBJECTS := $(OBJECTS)
TEST_OBJECTS += $(subst .cpp,.o, $(addprefix $(BUILD_DIR)/, $(test_src)))
TEST_OBJECTS += $(BUILD_DIR)/test_main.o

VPATH = ../src ../test $(CHI_SIM_HOME)/src

VERSION = 0.0
NAME = hepcep_model-$(VERSION)
TEST_NAME = unit_tests
TCL_LIB_NAME=libtcl$(NAME).$(LIB_EXT)

SED := sed
MV := mv -f

SWIFT_MODEL_DIR=../swift_proj/model

-include $(TEST_OBJECTS:.o=.d)
-include $(EXEC_OBJECTS:.o=.d)
-include $(LIB_OBJECTS:.o=.d)

.PHONY: all model tests clean

all: model tests

model : $(EXEC_OBJECTS)
	$(CXXLD) -fopenmp $(filter-out %.d, $^) $(LIBS) $(RPATHS) -o $(NAME)


hepcep_tcl_lib: $(LIB_OBJECTS) $(BUILD_DIR)/hepcep_model_wrapper.o
	$(CXXLD) -fopenmp $(filter-out %.d, $^) $(LIB_LDFLAGS) $(LIBS) $(RPATHS) $(TCL_LIB) -o $(SWIFT_MODEL_DIR)/$(TCL_LIB_NAME)
	LEAF_PKG=hepcep_model LEAF_VERSION=0.0 \
	LEAF_SO=$(TCL_LIB_NAME) LEAF_TCL=hepcep_model.tcl \
	$(TCLSH_LOCAL) $(SWIFT_MODEL_DIR)/make-package.tcl > $(SWIFT_MODEL_DIR)/pkgIndex.tcl

../src/hepcep_model_wrapper.cpp :  $(SWIFT_MODEL_DIR)/hepcep_model.i ../src/run.h
	$(SWIG) -c++ -o $(@) $(<)
	$(SED) -i.bu "s/Hepcep_model_Init/Tclhepcep_model_Init/" $(@)


tests : $(TEST_OBJECTS)
	$(CXXLD) $(filter-out %.d, $^) $(LIBS) $(RPATHS) $(GTEST_LIB) -lpthread -o $(TEST_NAME)

# -Wno-deprecated-declarations to turn off boost auto_ptr warnings
$(BUILD_DIR)/%.o : %.cpp
	@-mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -Wno-deprecated-declarations $(LIB_CXX_FLAGS) $(INCLUDES) $(TEST_INCLUDES) -c $< -o $@


clean:
	rm -fv $(NAME) $(TEST_NAME)
	rm -rf $(BUILD_DIR)/*
	rm -rf ../swift/lib/$(TCL_LIB_NAME)
