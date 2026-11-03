# Linux build: make -C Project1   (on SteamOS use ./build-steamos.sh instead)
# The binary goes in linux/ so that its asset paths ("../Project1/assets/...",
# relative to the executable) resolve the same way they do from Windows' Debug/ folder.
CXX ?= g++
CXXFLAGS ?= -O2 -g0
CXXFLAGS += -MMD -MP -std=c++17 $(shell sdl2-config --cflags) -w
LDLIBS += $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lpthread
LDFLAGS += -Wl,--disable-new-dtags,-rpath,'$$ORIGIN/lib'

SRCS := $(wildcard Project1/*.cpp)
OBJS := $(SRCS:Project1/%.cpp=build/%.o)

linux/acsf: $(OBJS)
	@mkdir -p linux
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: Project1/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf build linux/acsf

.PHONY: clean

-include $(OBJS:.o=.d)
