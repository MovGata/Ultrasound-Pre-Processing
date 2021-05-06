# Include paths
IPATHS = -isystem C:/msys64/mingw64/include/SDL2 -isystem C:/VulkanSDK/1.2.154.1/Include -isystem C:/VulkanSDK/VulkanMemoryAllocator/src
# Library paths
LPATHS = -LC:/msys64/mingw64/lib #-LC:/VulkanSDK/1.2.154.1/Lib

# Directories
DEPDIR = .d
OBJDIR = .o

# Make .d, .o DIR, not tested well, no subfolders.
$(@shell mkdir $(DEPDIR))
$(@shell mkdir $(OBJDIR))

# -Wl,-subsystem,windows gets rid of the console window
DEFS = -DWIN32_LEAN_AND_MEAN -DWINVER=0x0A00 -DVK_USE_PLATFORM_WIN32_KHR -DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 -DCPP_SDL2_VK_WINDOW -DCPP_SDL2_USE_SDL_IMAGE -DVMA_STATIC_VULKAN_FUNCTIONS=0 -DVMA_DYNAMIC_VULKAN_FUNCTIONS=0

# Windows FLAGS
WIN = -mconsole -mwindows
# G++ Flags
GPP = -std=c++2a -O0 -g -fconcepts -Werror -Wextra -Wall -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wcast-qual -Wswitch-enum -Wconversion -Wno-unknown-pragmas #-Wfatal-errors #-DDEBUG_Matrix #-DDEBUG_VECTOR3
# Linker flags
LINK = -g -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_TTF -L -lvulkan-1
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