#
# General Compiler Settings
#

CC=g++
EMSCRIPTEN=1

# general compiler settings
ifeq ($(EMSCRIPTEN),1)
        FLAGS= -s FULL_ES2=1 -I../gl4es/include -s USE_SDL=2
        FLAGS+= -I/opt/homebrew/opt/glm/include/glm
        FLAGS+= -Dlinux -DUSE_SDL2
        FLAGS+= --emrun
        FLAGS+= --shell-file template.html
        FLAGS+= -emit-llvm
        LDFLAGS= -s FULL_ES2=1 -s USE_SDL=2
        CC= emcc
        CXX= emc++
endif

FLAGS+= -pipe -fpermissive
CFLAGS=$(FLAGS) -Wno-conversion-null -Wno-write-strings -ICommon
LDFLAGS=$(FLAGS)
PROFILE=0

CFLAGS+=-O3 -Winit-self
LDFLAGS+=-s

GL4ES = ../gl4es/lib/libGL.a
LIB+= -lopenal ${GL4ES}

# specific includes
CFLAGS += -I.
CFLAGS += -DSOUND_OPENAL

BIN=docs/index.html

INC=$(wildcard *.h)
SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.bc,$(SRC))
#Not in OBJ to avoid removal with a "clean" command
INC+=${GL4ES}

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIB)

$(OBJ): $(INC)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

%.bc: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	$(RM) $(OBJ) $(BIN)

check:
	@echo
	@echo "INC = $(INC)"
	@echo
	@echo "SRC = $(SRC)"
	@echo
	@echo "OBJ = $(OBJ)"
	@echo
	@echo "PROFILE = $(PROFILE)"
	@echo
	@echo "CC = $(CC)"
	@echo "BIN = $(BIN)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "LIB = $(LIB)"
	@echo
