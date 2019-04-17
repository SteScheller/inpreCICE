# Target Platform: Linux

TARGET = inpreCICE
BUILD_DIR = build

SOURCES = src/main.cpp
SOURCES += src/draw/draw.cpp
SOURCES += src/util/util.cpp src/util/texture.cpp src/util/geometry.cpp
SOURCES += lib/imgui/imgui_impl_glfw.cpp lib/imgui/imgui_impl_opengl3.cpp
SOURCES += lib/imgui/imgui.cpp lib/imgui/imgui_demo.cpp
SOURCES += lib/imgui/imgui_draw.cpp lib/imgui/imgui_widgets.cpp
SOURCES += lib/gl3w/GL/gl3w.c
SOURCES += src/adapter/inpreciceadapter.cpp

OBJS = $(addsuffix .o, $(basename $(SOURCES)))

INCLUDE = -I./src -I./src/draw
INCLUDE += -I./include -I./lib/gl3w -I./lib/imgui -I./lib/nlohmann

CC = cc
CXX = g++
LINKER = ld

CXXFLAGS = $(INCLUDE) -std=c++14 `pkg-config --cflags glfw3` -fopenmp
CXXFLAGS += -Wall -Wextra
DEBUG_CXXFLAGS = -DDEBUG -g
RELEASE_CXXFLAGS = -DRELEASE -O3

CFLAGS = $(INCLUDE) -std=c11 `pkg-config --cflags glfw3` -fopenmp
CFLAGS += -Wall -Wextra
DEBUG_CFLAGS = -DDEBUG -g
RELEASE_CFLAGS = -DRELEASE -O3

LDFLAGS = -lGL `pkg-config --static --libs glfw3` -fopenmp
LDFLAGS += -lboost_program_options -lboost_system -lboost_iostreams
LDFLAGS += -lfreeimage
LDFLAGS += -lprecice

.PHONY: clean

default: debug

debug: CADDITIONALFLAGS = $(DEBUG_CFLAGS)
debug: CXXADDITIONALFLAGS = $(DEBUG_CXXFLAGS)
debug: TARGET_DIR = $(BUILD_DIR)/debug
debug: $(BUILD_DIR) $(BUILD_DIR)/debug start $(TARGET)
	@echo Build of standalone executable complete!

release: CADDITIONALFLAGS = $(RELEASE_CFLAGS)
release: CXXADDITIONALFLAGS = $(RELEASE_CXXFLAGS)
release: TARGET_DIR = $(BUILD_DIR)/release
release: $(BUILD_DIR) $(BUILD_DIR)/release start $(TARGET)
	@echo Build of standalone executable complete!

start:
	@echo -------------------------------------------------------------------------------
	@echo Compiling...
	@echo
	@echo CXXFLAGS: $(CXXFLAGS) $(CXXADDITIONALFLAGS)
	@echo CFLAGS: $(CFLAGS) $(CADDITIONALFLAGS)
	@echo TARGET_DIR $(TARGET_DIR)
	@echo

%.o: %.cpp
	@echo $<
	@$(CXX) $(CXXFLAGS) $(CXXADDITIONALFLAGS) -c -o $(TARGET_DIR)/$(@F) $<

%.o: %.c
	@echo $<
	@$(CC) $(CFLAGS)  $(CADDITIONALFLAGS) -c -o $(TARGET_DIR)/$(@F) $<

$(BUILD_DIR):
	@echo Creating build directory...
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%:
	@echo Creating target directory...
	@mkdir -p $@

$(TARGET): $(OBJS)
	@echo -------------------------------------------------------------------------------
	@echo Linking executable...
	@echo
	@echo LDFLAGS: $(LDFLAGS) $(LDADDITIONALFLAGS)
	@echo TARGET_DIR: $(TARGET_DIR)
	@echo TARGET: $(TARGET)
	@echo
	@$(CXX) $(addprefix $(TARGET_DIR)/, $(notdir $^)) $(LDFLAGS) $(LDADDITIONALFLAGS) -o $(TARGET_DIR)/$(TARGET)

clean:
	@echo Cleaning up...
	@rm -rf ./$(BUILD_DIR)/debug
	@rm -rf ./$(BUILD_DIR)/release
	@echo Done!

