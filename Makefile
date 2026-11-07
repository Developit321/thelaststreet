# The Last Street: gameplay prototype
#
#   make        build the prototype
#   make run    build and launch it
#   make clean  remove build output
#
# Needs only the tools that ship with Xcode (clang, make). No admin rights.

BUILD      := build
APP        := $(BUILD)/the-last-street

CXX        := clang++
CC         := clang

# --- raylib (window, drawing, input) --------------------------------------
RAYLIB_SRC := third_party/raylib/src
RAYLIB_OBJ := $(BUILD)/raylib
RAYLIB_LIB := $(RAYLIB_OBJ)/libraylib.a
RAYLIB_C   := rcore rshapes rtextures rtext rmodels raudio utils
RAYLIB_CFLAGS := -std=c99 -O2 -fno-strict-aliasing -Wno-missing-braces \
                 -Wno-deprecated-declarations -Wno-unused-function \
                 -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 \
                 -I$(RAYLIB_SRC) -I$(RAYLIB_SRC)/external/glfw/include

# --- our code -------------------------------------------------------------
# core/    the simulation: plain C++, no graphics, no exceptions (moves to Unreal)
# loader/  reads data/ files into core types (replaced by Unreal's JSON later)
# app/     the throwaway raylib front end
CORE_SRC   := $(wildcard core/src/*.cpp)
LOADER_SRC := $(wildcard loader/*.cpp)
APP_SRC    := $(wildcard app/*.cpp)

CORE_OBJ   := $(CORE_SRC:%.cpp=$(BUILD)/obj/%.o)
LOADER_OBJ := $(LOADER_SRC:%.cpp=$(BUILD)/obj/%.o)
APP_OBJ    := $(APP_SRC:%.cpp=$(BUILD)/obj/%.o)

CXXFLAGS_COMMON := -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -MMD -MP \
                   -Icore/include
CORE_CXXFLAGS   := $(CXXFLAGS_COMMON) -fno-exceptions
LOADER_CXXFLAGS := $(CXXFLAGS_COMMON) -Ithird_party
APP_CXXFLAGS    := $(CXXFLAGS_COMMON) -Iloader -I$(RAYLIB_SRC) -Wno-missing-field-initializers

FRAMEWORKS := -framework Cocoa -framework IOKit -framework CoreVideo \
              -framework OpenGL -framework CoreAudio -framework AudioToolbox

.PHONY: all run clean
all: $(APP)

run: $(APP)
	./$(APP)

$(APP): $(CORE_OBJ) $(LOADER_OBJ) $(APP_OBJ) $(RAYLIB_LIB)
	$(CXX) $^ $(FRAMEWORKS) -o $@

$(BUILD)/obj/core/%.o: core/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CORE_CXXFLAGS) -c $< -o $@

$(BUILD)/obj/loader/%.o: loader/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(LOADER_CXXFLAGS) -c $< -o $@

$(BUILD)/obj/app/%.o: app/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(APP_CXXFLAGS) -c $< -o $@

$(RAYLIB_LIB): $(RAYLIB_C:%=$(RAYLIB_OBJ)/%.o) $(RAYLIB_OBJ)/rglfw.o
	ar rcs $@ $^

$(RAYLIB_OBJ)/%.o: $(RAYLIB_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(RAYLIB_CFLAGS) -c $< -o $@

# GLFW's macOS backend is Objective-C.
$(RAYLIB_OBJ)/rglfw.o: $(RAYLIB_SRC)/rglfw.c
	@mkdir -p $(dir $@)
	$(CC) $(RAYLIB_CFLAGS) -x objective-c -c $< -o $@

clean:
	rm -rf $(BUILD)

-include $(shell find $(BUILD)/obj -name '*.d' 2>/dev/null)
