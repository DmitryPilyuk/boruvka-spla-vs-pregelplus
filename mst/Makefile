# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = 

# Project paths
SRCDIR = src
BUILDDIR = build
TARGET = mst

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

SPLA_DIR = ../deps/SPLA
SPLA_INCLUDE = -I$(SPLA_DIR)/include
SPLA_LIB_DIR = -L$(SPLA_DIR)/build
SPLA_LIBS = -lspla_x64

# Add SPLA includes and libraries to compiler and linker flags
CXXFLAGS += $(SPLA_INCLUDE)
LDFLAGS += $(SPLA_LIB_DIR) $(SPLA_LIBS)

# Executable
EXECUTABLE = $(BUILDDIR)/$(TARGET)

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(SPLA_DIR)/build/libspla_x64.so
	@mkdir -p $(@D)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SPLA_DIR)/build/libspla_x64.so:
	cmake $(SPLA_DIR) -B $(SPLA_DIR)/build/ -G Ninja -DCMAKE_BUILD_TYPE=Release -DSPLA_BUILD_OPENCL=YES -DSPLA_BUILD_TESTS=NO -DSPLA_BUILD_EXAMPLES=NO
	cmake --build $(SPLA_DIR)/build/ --target all
  
clean:
	rm -rf $(BUILDDIR)

distclean: clean
	rm -rf $(SPLA_DIR)/build

