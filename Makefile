# Black Shades - build for modern macOS (and Linux, which still works fine)
#
#   make              build ./blackshades for this machine
#   make app          build BlackShades.app for this Mac (arm64 on Apple
#                     Silicon); 'make app UNIVERSAL=1' for arm64 + x86_64
#   make run          build and run
#   make clean
#
# macOS needs only the Xcode Command Line Tools: `make app` downloads the
# official universal SDL2.framework into macos/Frameworks the first time and
# bundles it into the .app.
#
# Options:
#   UNIVERSAL=1           build a fat arm64 + x86_64 .app instead of just this
#                         machine's architecture
#   ARCHS="arm64"         pick the .app's architectures explicitly
#   USE_SYSTEM_SDL2=1     build against the SDL2 already installed (Homebrew,
#                         MacPorts, a distro package) instead of the bundled
#                         framework; that dylib is copied into the bundle
#   USE_SYSTEM_OPENAL=1   link the platform's OpenAL instead of the built-in
#                         MiniAL mixer (OpenAL.framework on macOS, -lopenal
#                         elsewhere)
#   ARCHFLAGS="-arch ..." cross/universal builds (used by macos/build-app.sh)
#   OBJDIR=...            where object files go

UNAME_S := $(shell uname -s)

SRCDIR  := Source
OBJDIR  ?= build
EXE     ?= blackshades

CXX ?= c++
CC  ?= cc

WARNINGS := -Wall -Wno-unused-variable -Wno-unused-but-set-variable \
            -Wno-unused-value -Wno-parentheses -Wno-write-strings \
            -Wno-sign-compare -Wno-format-security -Wno-misleading-indentation \
            -Wno-deprecated-declarations

BASEFLAGS := -O2 -g $(ARCHFLAGS) $(WARNINGS) -I$(SRCDIR) -DSDL_MAIN_HANDLED

# ---------------------------------------------------------------- finding SDL2

ifdef USE_SYSTEM_SDL2
  SDL2_FRAMEWORK :=
else
  SDL2_FRAMEWORK := $(wildcard macos/Frameworks/SDL2.framework)
endif

ifneq ($(SDL2_FRAMEWORK),)
  SDL_CFLAGS := -Fmacos/Frameworks -Imacos/Frameworks/SDL2.framework/Headers
  SDL_LIBS   := -Fmacos/Frameworks -framework SDL2
else
  SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null || pkg-config --cflags sdl2 2>/dev/null)
  SDL_LIBS   := $(shell sdl2-config --libs 2>/dev/null || pkg-config --libs sdl2 2>/dev/null)

  # Last resort: look in the usual prefixes, so an unusual Homebrew root or a
  # missing PKG_CONFIG_PATH is not a dead end.
  ifeq ($(strip $(SDL_LIBS)),)
    SDL_PREFIX := $(firstword $(foreach p,$(HOME)/.homebrew /opt/homebrew /usr/local /opt/local,\
                    $(if $(wildcard $(p)/include/SDL2/SDL.h),$(p))))
    ifneq ($(SDL_PREFIX),)
      SDL_CFLAGS := -I$(SDL_PREFIX)/include/SDL2 -D_THREAD_SAFE
      SDL_LIBS   := -L$(SDL_PREFIX)/lib -lSDL2
    endif
  endif
endif

# ---------------------------------------------------------------- platform

ifeq ($(UNAME_S),Darwin)
  GL_LIBS := -framework OpenGL
  ifdef USE_SYSTEM_OPENAL
    AUDIO_LIBS := -framework OpenAL
  endif
  PLATFORM_LIBS := -framework Cocoa
  # let the .app find SDL2 (framework or dylib) inside Contents/Frameworks
  RPATHS := -Wl,-rpath,@executable_path/../Frameworks -Wl,-rpath,@loader_path/macos/Frameworks
else
  GL_LIBS := -lGL
  ifdef USE_SYSTEM_OPENAL
    AUDIO_LIBS := -lopenal
  endif
  PLATFORM_LIBS := -lm
  RPATHS :=
endif

ifdef USE_SYSTEM_OPENAL
  BASEFLAGS += -DUSE_SYSTEM_OPENAL=1
endif

# -MMD -MP: every object records the headers it included (build/*.d), so
# editing a header - say, adding a field to Person - rebuilds everything that
# uses it instead of linking objects built for the old layout.
DEPFLAGS := -MMD -MP

CXXFLAGS := $(BASEFLAGS) $(SDL_CFLAGS) $(DEPFLAGS) -include $(SRCDIR)/Support.h
CFLAGS   := -O2 -g $(ARCHFLAGS) -Wall -Wno-unused-value $(SDL_CFLAGS) $(DEPFLAGS)
LDFLAGS  += $(ARCHFLAGS) $(RPATHS)
LIBS     := $(SDL_LIBS) $(GL_LIBS) $(AUDIO_LIBS) $(PLATFORM_LIBS)

# ---------------------------------------------------------------- sources
#
# AGL_DSp.cpp, Alerts.cpp and Files.cpp are the Mac OS 9 originals; the SDL
# versions of what they did live in Support.cpp.

CXXSRCS := \
	Camera.cpp \
	Decals.cpp \
	Fog.cpp \
	Frustum.cpp \
	GameDraw.cpp \
	GameInitDispose.cpp \
	GameLoop.cpp \
	GameTick.cpp \
	Globals.cpp \
	MacInput.cpp \
	Main.cpp \
	MiniAL.cpp \
	Maths.cpp \
	Models.cpp \
	Person.cpp \
	Quaternions.cpp \
	Serialize.cpp \
	Skeleton.cpp \
	Sprites.cpp \
	Support.cpp \
	TGALoader.cpp \
	Text.cpp \
	Textures.cpp \
	Timer.cpp

CSRCS := \
	thirdparty/stb_image_impl.c \
	thirdparty/stb_vorbis.c

ifdef USE_SYSTEM_OPENAL
  CXXSRCS := $(filter-out MiniAL.cpp,$(CXXSRCS))
endif

OBJS := $(addprefix $(OBJDIR)/,$(CXXSRCS:.cpp=.o) $(CSRCS:.c=.o))

# ---------------------------------------------------------------- rules

.DEFAULT_GOAL := all
.PHONY: all app run clean distclean sdl2

all: $(EXE)

$(EXE): $(OBJS)
ifeq ($(strip $(SDL_LIBS)),)
	@echo "*** No SDL2 found."; \
	 echo "***   macOS: run 'make sdl2' to fetch the official framework,"; \
	 echo "***          or 'brew install sdl2' and build with USE_SYSTEM_SDL2=1"; \
	 echo "***   Linux: install libsdl2-dev"; \
	 exit 1
endif
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Grab the official universal SDL2.framework (macOS only, no-op if present)
sdl2:
	@sh macos/get-sdl2.sh

# build-app.sh fetches SDL2 itself unless USE_SYSTEM_SDL2 is set
app:
	@sh macos/build-app.sh

run: all
	./$(EXE)

clean:
	rm -rf $(OBJDIR) $(EXE)

distclean: clean
	rm -rf BlackShades.app macos/Frameworks build-arm64 build-x86_64 \
	       blackshades-arm64 blackshades-x86_64

# ---------------------------------------------------------------- dependencies

-include $(OBJS:.o=.d)

# changing the Makefile (flags, sources) rebuilds everything too
$(OBJS): Makefile
