#paths for source files, object files, and executable
SRC_PATH = src
BUILD_PATH = obj
BIN_PATH = bin


SRCS = $(shell find $(SRC_PATH) -name '*.cpp')
EXT_FILES = src/externals/pd~.c   #need to compile the external pd~.c for multithreadig

CXX = g++ #c++ compiler to use
TARGET = $(BIN_PATH)/Lilikoi # name of output executable file

# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJS = $(SRCS:$(SRC_PATH)/%.cpp=$(BUILD_PATH)/%.o)
OBJS += obj/pd~.o

LIBPD_DIR = /home/samlan/libpd
LIBPD = $(LIBPD_DIR)/libs/libpd.so

LDLIBS = $(shell pkg-config --libs opencv) $(LIBPD)

COMPILEFLAGS = #add -pg to this and CFLAGS to use gprof profiling

CFLAGS = -Wall
CFLAGS += $(shell pkg-config --cflags opencv)
CFLAGS += -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper \
            -I$(LIBPD_DIR)/libpd_wrapper/util -I$(LIBPD_DIR)/cpp -I./src -O3
CFLAGS += -DPD  #added these for externals
CXXFLAGS = $(CFLAGS) -std=c++11 -DLIBPD_USE_STD_MUTEX  #added these for externals

#stuff for teh linuxes audio (Change this if compiling for another system. See pdtest_rtaudio's Makefile)
CFLAGS += -D__UNIX_JACK__ -D__LINUX_ALSA__
AUDIO_API = -ljack -lasound -pthread

# Gcc will create these .d files containing dependencies.
DEP = $(OBJS:%.o=%.d)

default: all

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(COMPILEFLAGS) $(LDLIBS) $(AUDIO_API)

# Include all .d files
-include $(DEP)

obj/pd~.o: $(EXT_FILES)
	gcc $(CFLAGS) -MMD -c -o $(BUILD_PATH)/pd~.o src/externals/pd~.c

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c -o $@ $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)
	rm -f $(DEP)