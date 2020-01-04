#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
#

#CXX = g++-8 -std=c++17
CXX = clang++-8
CC = clang-8

EXE = SourceExplorer
SOURCES = src/main.cpp libs/text_editor/TextEditor.cpp src/graph.cpp src/clang_interface.cpp src/gui.cpp
SOURCES += libs/imgui/glfw_opengl3/imgui_impl_glfw.cpp libs/imgui/glfw_opengl3/imgui_impl_opengl3.cpp
SOURCES += libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_widgets.cpp libs/imgui/misc/cpp/imgui_stdlib.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

LLVMCOMPONENTS := cppbackend
RTTIFLAG := -fno-rtti
LLVMCONFIG := /usr/bin/llvm-config-8

INCLUDE = -Ilibs/imgui/glfw_opengl3/
INCLUDE += -Ilibs/imgui/
INCLUDE += -I$(shell $(LLVMCONFIG) --src-root)/tools/clang/include
INCLUDE += -I$(shell $(LLVMCONFIG) --obj-root)/tools/clang/include
INCLUDE += -Ilibs/text_editor/
INCLUDE += -Ilibs/imgui/misc/cpp/


CXXFLAGS =  $(shell $(LLVMCONFIG) --cxxflags) $(RTTIFLAG) -std=c++17 -g -Wall -Wformat
CFLAGS = -std=c99

LIBS = \
				-lclangTooling\
				-lclangFrontendTool\
				-lclangFrontend\
				-lclangDriver\
				-lclangSerialization\
				-lclangCodeGen\
				-lclangParse\
				-lclangSema\
				-lclangStaticAnalyzerFrontend\
				-lclangStaticAnalyzerCheckers\
				-lclangStaticAnalyzerCore\
				-lclangAnalysis\
				-lclangARCMigrate\
				-lclangRewrite\
				-lclangRewriteFrontend\
				-lclangEdit\
				-lclangAST\
				-lclangLex\
				-lclangBasic\
				-lclangAST\
				-lclangASTMatchers\
				$(shell $(LLVMCONFIG) --libs)\
				$(shell $(LLVMCONFIG) --system-libs)\
				-lcurses\
				-lstdc++fs\
				-lGLEW

#LIBS =

##---------------------------------------------------------------------
## OPENGL LOADER
##---------------------------------------------------------------------

## Using OpenGL loader: gl3w [default]
#SOURCES += imgui_util/glfw_opengl3/libs/gl3w/GL/gl3w.c
#CXXFLAGS += -Iimgui_util/glfw_opengl3/libs/gl3w

## Using OpenGL loader: glew
## (This assumes a system-wide installation)
CXXFLAGS += -DIMGUI_IMPL_OPENGL_LOADER_GLEW

## Using OpenGL loader: glad
# SOURCES += ../libs/glad/src/glad.c
# CXXFLAGS += -I../libs/glad/include -DIMGUI_IMPL_OPENGL_LOADER_GLAD

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += -lGL -lGLEW `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
#	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:src/%.cpp
	$(CXX) $(INCLUDE) $(LLVMDFLAGS) $(CXXFLAGS)  -c -o $@ $<

%.o:libs/imgui/glfw_opengl3/%.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) -c -o $@ $<

%.o:libs/imgui/%.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) -c -o $@ $<

%.o:libs/text_editor/%.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) -c -o $@ $<

%.o:libs/imgui/glfw_opengl3/libs/gl3w/GL/%.c
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

%.o:libs/imgui/misc/cpp/imgui_stdlib.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(EXE)