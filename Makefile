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

EXE = SourceExplorer
SOURCES = src/main.cpp libs/text_editor/TextEditor.cpp src/graph.cpp src/clang_interface.cpp src/gui.cpp
SOURCES += libs/imgui/glfw_opengl3/imgui_impl_glfw.cpp libs/imgui/glfw_opengl3/imgui_impl_opengl3.cpp
SOURCES += libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_widgets.cpp 

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
				-lclangParse\
				-lclangSema\
				-lclangStaticAnalyzerFrontend\
				-lclangStaticAnalyzerCheckers\
				-lclangStaticAnalyzerCore\
				-lclangAnalysis\
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
				-lGLEW\
				-lGL\
				`pkg-config --static --libs glfw3`\

CXXFLAGS += -DIMGUI_IMPL_OPENGL_LOADER_GLEW
CXXFLAGS += `pkg-config --cflags glfw3`




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

all: $(EXE)
	chmod 0444 imgui.ini
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PRECIOUS: %.o Makefile

.PHONY: clean

clean:
	rm -f $(OBJS) $(EXE)

