#paths for source files, object files, and executable
SRC_PATH = src
BUILD_PATH = obj
BIN_PATH = bin

# extensions #
SRC_EXT = cpp

SRCS = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)')

CXX = g++ #c++ compiler to use
TARGET = $(BIN_PATH)/ReverseVisualizer # name of output executable file

# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJS = $(SRCS:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)

LIBPD_DIR = /home/samlan/libpd
LIBPD = $(LIBPD_DIR)/libs/libpd.so

LDLIBS = $(shell pkg-config --libs opencv) $(LIBPD)

CXXFLAGS = -Wall 
CXXFLAGS += $(shell pkg-config --cflags opencv)
CXXFLAGS += -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper \
            -I$(LIBPD_DIR)/libpd_wrapper/util -I$(LIBPD_DIR)/cpp -I./src -O3

#stuff for teh linuxes audio (Change this if compiling for another system. See pdtest_rtaudio's Makefile)
CXXFLAGS += -D__UNIX_JACK__ -D__LINUX_ALSA__
AUDIO_API = -ljack -lasound -pthread

default: all

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDLIBS) $(AUDIO_API)
	#$@ means the target (in this case, $(TARGET) )

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT) #$(OBJS): $(SRCS) #maybe change to $(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)