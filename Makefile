## GLFW and GLAD paths include
INCLUDE_DIRS =  -I"C:\GLAD\include"\
				-I"C:\GLFW\include"\
				-I"inc"

LIB_DIRS =  -L"C:\GLAD\lib"\
			-L"C:\GLFW\lib-mingw-w64"

## Compiler
CXX = g++

EXE = bin/GUI_for_platform_transport.exe

## Project source files
SOURCES = src/main.cpp

## ImGui source files
IMGUI_DIR = C:\ImGui
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

## ImPlot source files
SOURCES += $(IMGUI_DIR)/implot.cpp $(IMGUI_DIR)/implot_demo.cpp $(IMGUI_DIR)/implot_items.cpp

## Object files
OBJS = $(addprefix obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))

## Compiler flags
CXXFLAGS = -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat
CXXFLAGS += ${INCLUDE_DIRS}

LIBS += -lglad -lglfw3 -lgdi32 -lopengl32 -limm32 -lpthread
LIBS += ${LIB_DIRS}

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

.PHONY: all clean

all: $(EXE)
	@echo ***BUILD COMPLETE***

obj:
	@if not exist obj mkdir obj
bin:
	@if not exist bin mkdir bin

obj/%.o:src/%.cpp | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/%.o:$(IMGUI_DIR)/%.cpp | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/%.o:$(IMGUI_DIR)/backends/%.cpp | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXE): $(OBJS) | bin
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	if exist obj\*.o        del   obj\*.o
	if exist bin\*.exe      del   bin\*.exe
	if exist bin\*.dll      del   bin\*.dll
	if exist bin\*.ini      del   bin\*.ini
	if exist obj            rmdir obj
	if exist bin            rmdir bin
