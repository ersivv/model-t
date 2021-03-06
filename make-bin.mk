##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

include deps.mk

# Compiler options here.
ifeq ($(CONFIG),release)
  USE_OPT = -Os -fomit-frame-pointer -falign-functions=16
else
  USE_OPT = -O0 -ggdb
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = -MMD
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Enable this if you really want to use the STM FWLib.
ifeq ($(USE_FWLIB),)
  USE_FWLIB = no
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

PROJECT_SRC_DIR = src/$(PROJECT)
BUILDDIR   = build/$(PROJECT)
AUTOGEN_DIR = $(BUILDDIR)/autogen

# Imported source files and paths
include board/board.mk
include $(CHIBIOS)/os/hal/platforms/STM32F4xx/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/ports/GCC/ARMCMx/STM32F2xx/port.mk
include $(CHIBIOS)/os/kernel/kernel.mk

# Define linker script file here
LDSCRIPT= $(PROJECT_SRC_DIR)/$(PROJECT).ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(PORTSRC) \
       $(KERNSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(CHIBIOS)/os/various/evtimer.c \
       $(addprefix $(AUTOGEN_DIR)/,$(PROJECT_AUTOGEN_CSRC)) \
       $(addprefix $(PROJECT_SRC_DIR)/,$(PROJECT_CSRC)) \
       $(foreach dep,$(addsuffix _CSRC,$(DEPS)),$($(dep)))

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(addprefix $(PROJECT_SRC_DIR)/,$(PROJECT_CPPSRC))

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC = $(addprefix $(PROJECT_SRC_DIR)/,$(PROJECT_ACSRC))

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC = $(addprefix $(PROJECT_SRC_DIR)/,$(PROJECT_ACPPSRC))

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC = $(addprefix $(PROJECT_SRC_DIR)/,$(PROJECT_TCSRC))

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC = $(addprefix $(PROJECT_SRC_DIR)/,$(PROJECT_TCPPSRC))

# List ASM source files here
ASMSRC = $(PORTASM) \
         $(addprefix $(PROJECT_SRC_DIR)/,$(PROJECT_ASMSRC))

INCDIR = $(PORTINC) $(KERNINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) \
         $(CHIBIOS)/os/various \
         $(foreach dep,$(addsuffix _INCDIR,$(DEPS)),$($(dep)))

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m3

#TRGT = arm-elf-
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra

#
# Compiler settings
##############################################################################

##############################################################################
# Start of default section
#

# List all default C defines here, like -D_DEBUG=1
DDEFS = -DREQUIRE_PRINTF_FLOAT -D__DYNAMIC_REENT__

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = -lm --specs=nano.specs
ifeq ($(CONFIG),debug)
DLIBS += --specs=rdimon.specs -lc -lc -lrdimon
DDEFS += -DUSE_SEMIHOSTING -DDEBUG
endif

#
# End of default section
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = -DMAJOR_VERSION=$(MAJOR_VERSION) \
        -DMINOR_VERSION=$(MINOR_VERSION) \
        -DPATCH_VERSION=$(PATCH_VERSION) \
        -DVERSION_STR=\"$(MAJOR_VERSION).$(MINOR_VERSION).$(PATCH_VERSION)\" \
        -DWEB_API_HOST=$(WEB_API_HOST) \
        -DWEB_API_PORT=$(WEB_API_PORT) \
         $(foreach dep,$(addsuffix _DEFS,$(DEPS)),$($(dep)))

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR = src/common \
          $(AUTOGEN_DIR) \
          $(PROJECT_SRC_DIR) \
          $(addprefix $(PROJECT_SRC_DIR)/,$(PROJECT_INCDIR))
          
# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user defines
##############################################################################

ifeq ($(USE_FWLIB),yes)
  include $(CHIBIOS)/ext/stm32lib/stm32lib.mk
  CSRC += $(STM32SRC)
  INCDIR += $(STM32INC)
  USE_OPT += -DUSE_STDPERIPH_DRIVER
endif

include $(CHIBIOS)/os/ports/GCC/ARMCMx/rules.mk

AUTOGEN_SRCS = \
	font_resources.c \
	image_resources.c \
	bbmt.pb.c

autogen: $(addprefix $(AUTOGEN_DIR)/, $(AUTOGEN_SRCS)) | $(AUTOGEN_DIR)

$(AUTOGEN_DIR): | $(BUILDDIR)
	@mkdir -p $@

$(AUTOGEN_DIR)/font_resources.c $(AUTOGEN_DIR)/font_resources.h: scripts/fontconv $(wildcard fonts/*.ttf) fonts/font_specs | $(AUTOGEN_DIR)
	@python scripts/fontconv fonts $(AUTOGEN_DIR)

$(AUTOGEN_DIR)/image_resources.c $(AUTOGEN_DIR)/image_resources.h: scripts/imgconv $(wildcard images/*.png) | $(AUTOGEN_DIR)
	@python scripts/imgconv $(AUTOGEN_DIR) $(wildcard images/*.png)

$(AUTOGEN_DIR)/bbmt.pb: $(BBMT_MSGS)/bbmt.proto | $(AUTOGEN_DIR)
	@protoc $(BBMT_MSGS_INCLUDES) -o$@ --python_out=$(AUTOGEN_DIR) $(BBMT_MSGS)/bbmt.proto
	
$(AUTOGEN_DIR)/bbmt.pb.c $(AUTOGEN_DIR)/bbmt.pb.h: $(AUTOGEN_DIR)/bbmt.pb | $(AUTOGEN_DIR)
	@python $(NANOPB)/generator/nanopb_generator.py $(AUTOGEN_DIR)/bbmt.pb

