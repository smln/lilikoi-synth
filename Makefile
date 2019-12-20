#paths for source files, object files, and executable
SRC_PATH = src
EXT_PATH = src/externals
BUILD_PATH = obj
BIN_PATH = bin


SRCS = $(shell find $(SRC_PATH) -name '*.cpp')
EXT_SRCS = $(shell find $(EXT_PATH) -name '*.c')
#EXT_SRCS = src/externals/pd~.c   #need to compile the external pd~.c for multithreadig

CXX = g++ #c++ compiler to use
TARGET = $(BIN_PATH)/Lilikoi # name of output executable file

# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJS = $(SRCS:$(SRC_PATH)/%.cpp=$(BUILD_PATH)/%.o)
OBJS += $(EXT_SRCS:$(EXT_PATH)/%.c=$(EXT_PATH)/%.o)

LIBPD_DIR = /home/samlan/libpd
LIBPD = $(LIBPD_DIR)/libpd.so

LDLIBS = $(shell pkg-config --libs opencv) $(LIBPD)

COMPILEFLAGS = #add -pg to this and CFLAGS to use gprof profiling

CFLAGS = -Wall
CFLAGS += -g #@@@for debugging
CFLAGS += $(shell pkg-config --cflags opencv)
CFLAGS += -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper \
            -I$(LIBPD_DIR)/libpd_wrapper/util -I$(LIBPD_DIR)/cpp -I./src -I../../../cpp -I./src -O3
CFLAGS += -DPD -DLIBPD_USE_STD_MUTEX #added these for externals

#stuff for teh linuxes audio (Change this if compiling for another system. See pdtest_rtaudio's Makefile)
CFLAGS += -D__UNIX_JACK__ -D__LINUX_ALSA__

CXXFLAGS = $(CFLAGS) -std=c++11 

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

#$(BUILD_PATH)/%.o: $(EXT_PATH)/%.c
#	gcc $(CFLAGS) -MMD -c -o $@ $<

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c -o $@ $<

$(EXT_PATH)/%.o: $(EXT_PATH)/%.c
	mkdir -p $(@D)
	gcc $(CFLAGS) -MMD -c -o $@ $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)
	rm -f $(DEP)