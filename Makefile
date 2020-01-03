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
CXX = clang++-8 -std=c++17

EXE = SourceExplorer
SOURCES = main.cpp editor_util/editor_util.cpp editor_util/TextEditor.cpp graph.cpp clang_interface.cpp
SOURCES += imgui_util/glfw_opengl3/imgui_impl_glfw.cpp imgui_util/glfw_opengl3/imgui_impl_opengl3.cpp
SOURCES += imgui_util/imgui.cpp imgui_util/imgui_draw.cpp imgui_util/imgui_widgets.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

LLVMCOMPONENTS := cppbackend
RTTIFLAG := -fno-rtti
LLVMCONFIG := /usr/bin/llvm-config-8



CXXFLAGS = -Iimgui_util/glfw_opengl3/ -Iimgui_util/ -I$(shell $(LLVMCONFIG) --src-root)/tools/clang/include -I$(shell $(LLVMCONFIG) --obj-root)/tools/clang/include $(shell $(LLVMCONFIG) --cxxflags) $(RTTIFLAG) -std=c++17
CXXFLAGS += -lstdc++fs -g -Wall -Wformat 

CLANGLIBS = \
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
	-lcurses

#LIBS =

##---------------------------------------------------------------------
## OPENGL LOADER
##---------------------------------------------------------------------

## Using OpenGL loader: gl3w [default]
SOURCES += imgui_util/glfw_opengl3/libs/gl3w/GL/gl3w.c
CXXFLAGS += -Iimgui_util/glfw_opengl3/libs/gl3w

## Using OpenGL loader: glew
## (This assumes a system-wide installation)
# CXXFLAGS += -lGLEW -DIMGUI_IMPL_OPENGL_LOADER_GLEW

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
	CFLAGS = $(CXXFLAGS)
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

%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(LLVMDFLAGS) -c -o $@ $<

%.o:imgui_util/glfw_opengl3/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:imgui_util/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:editor_util/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:imgui_util/glfw_opengl3/libs/gl3w/GL/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o:imgui_util/glfw_opengl3/libs/glad/src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS) $(CLANGLIBS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(EXE)
