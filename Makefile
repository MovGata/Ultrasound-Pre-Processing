# Include paths
IPATHS = -isystem C:/msys64/mingw64/include/SDL2 -isystem C:/SDK
# Library paths
LPATHS = -LC:/msys64/mingw64/lib

# Directories
DEPDIR = .d
OBJDIR = .o

# Make .d, .o DIR, not tested well, no subfolders.
$(@shell mkdir $(DEPDIR))
$(@shell mkdir $(OBJDIR))

# -Wl,-subsystem,windows gets rid of the console window
DEFS = -DWIN32_LEAN_AND_MEAN -DWINVER=0x0A00 -DCPP_SDL2_VK_WINDOW -DCPP_SDL2_USE_SDL_IMAGE -D_USE_MATH_DEFINES -DCL_TARGET_OPENCL_VERSION=120 -DCL_HPP_TARGET_OPENCL_VERSION=120 -DCL_HPP_MINIMUM_OPENCL_VERSION=120 -DGL_GLEXT_PROTOTYPES -DCL_HPP_ENABLE_EXCEPTIONS -DGLM_FORCE_CXX2A

# Windows FLAGS
WIN = -mconsole -mwindows -Wl,-subsystem,windows
# G++ Flags
GPP = -std=c++2a -s -O2 -fconcepts -Werror -Wextra -Wall -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wcast-qual -Wswitch-enum -Wconversion -Wno-unknown-pragmas -fconcepts-diagnostics-depth=2 -Wa,-mbig-obj#-Wfatal-errors #-DDEBUG_Matrix #-DDEBUG_VECTOR3
# Linker flags
LINK = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_TTF -lglew32 -lopengl32 -lOpenCL
# Dependency flags
DEP = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

# Exectuable name
OBJ = test

POST = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

CXX = g++
# SRCS = $(wildcard **/*.cc)

vpath %.cc ./src

# Find all cc files that don't end with _test
SRCS := $(shell wsl find -name *.cc ! -name *_test.cc)
OBJS := $(patsubst ./src/%.cc,.o/%.o,$(SRCS))
DEPS := $(patsubst ./src/%.cc,.d/%.d,$(SRCS))

$(objects): | .o

.o:
	$(@shell mkdir $(OBJDIR))

all: $(OBJS)
	$(CXX) $(WIN) $(GPP) $(DEFS) $^ $(LINK) -o $(OBJ)

.o/%.o: %.cc
	$(CXX) $(IPATHS) $(WIN) $(GPP) $(DEFS) $(DEP) -c $< -o $@
	$(POST)

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(OBJS) $(DEPS)

-include $(DEPS)