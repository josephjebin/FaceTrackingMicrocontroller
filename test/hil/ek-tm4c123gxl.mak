##############################################################################
# Product: Makefile for ET (embedded test) on EK-TM4C123GXL, GNU-ARM
#
#                    Q u a n t u m  L e a P s
#                    ------------------------
#                    Modern Embedded Software
#
# Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
##############################################################################
# examples of invoking this Makefile:
# make -f ek-tm4c123gxl.mak
# make -f ek-tm4c123gxl.mak clean
# make -f ek-tm4c123gxl.mak flash
#
# NOTE:
# To use this Makefile on Windows, you will need the GNU make utility, which
# is included in the QTools collection for Windows, see:
#    https://github.com/QuantumLeaps/qtools
#

#-----------------------------------------------------------------------------
# project and target names
#
PROJECT := test
TARGET  := ek-tm4c123gxl

#-----------------------------------------------------------------------------
# project directories
#
TARGET_DIR := ./tm4c
UNITY_DIR := ../framework/src
CMOCK_DIR := C:\Users\josep\cmock\src
CMSIS_DIR := ../../CMSIS/Core/Include

# list of all source directories used by this project
VPATH := ./ \
	./mocks \
	../../src \
	./tm4c \
	../../tm4c \
	../../tm4c/system \
	$(UNITY_DIR) \
	$(CMOCK_DIR)

# list of all include directories needed by this project (source, header, and assembly files)
INCLUDES := -I. \
	-I./mocks \
	-I../../src \
	-I./tm4c \
	-I../../tm4c \
	-I../../tm4c/system \
	-I$(UNITY_DIR) \
	-I$(CMOCK_DIR) \
	-I$(CMSIS_DIR)

#-----------------------------------------------------------------------------
# project files
#

# assembler source files ./../../tm4c/startup/startup_TM4C123.s
ASM_SRCS := 

# C source files
C_SRCS := Mocktm4c_bsp_mocks.c \
	cyber_minion_hil_tests.c \
	cyber_minion.c \
	tm4c_bsp.c \
	system_TM4C123GH6PM.c \
	startup_TM4C123GH6PM.c \
	unity.c \
	cmock.c \
	
# C++ source files
CPP_SRCS :=

LD_SCRIPT  := $(TARGET_DIR)/$(TARGET).ld

OUTPUT    := $(PROJECT)

LIB_DIRS  :=
LIBS      :=

# defines
DEFINES   := -DTARGET_IS_TM4C123_RB1

# ARM CPU, ARCH, FPU, and Float-ABI types...
# ARM_CPU:   [cortex-m0 | cortex-m0plus | cortex-m1 | cortex-m3 | cortex-m4]
# ARM_FPU:   [ | vfp]
# FLOAT_ABI: [ | soft | softfp | hard]
#
ARM_CPU   := -mcpu=cortex-m4
ARM_FPU   := -mfpu=vfp
FLOAT_ABI := -mfloat-abi=softfp

#-----------------------------------------------------------------------------
# GNU-ARM toolset (NOTE: You need to adjust to your machine)
# see https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
#
GNU_ARM := C:/qp/qtools/gnu_arm-none-eabi

# make sure that the GNU-ARM toolset exists...
ifeq ("$(wildcard $(GNU_ARM))","")
$(error GNU_ARM toolset not found. Please adjust the Makefile)
endif

CC    := $(GNU_ARM)/bin/arm-none-eabi-gcc
CPP   := $(GNU_ARM)/bin/arm-none-eabi-g++
AS    := $(GNU_ARM)/bin/arm-none-eabi-as
LINK  := $(GNU_ARM)/bin/arm-none-eabi-gcc
BIN   := $(GNU_ARM)/bin/arm-none-eabi-objcopy

#-----------------------------------------------------------------------------
# NOTE: The following symbol LMFLASH assumes that LMFlash.exe can
# be found on the PATH. You might need to adjust this symbol to the
# location of the LMFlash utility on your machine
#
ifeq ($(LMFLASH),)
FLASH := LMFlash.exe -q ek-tm4c123gxl
endif

##############################################################################
# Typically you should not need to change anything below this line

# basic utilities (included in QTools for Windows), see:
#     https://www.state-machine.com/qtools

MKDIR := mkdir
RM    := rm

#-----------------------------------------------------------------------------
# build options
#

# combine all the soruces...
C_SRCS += $(QP_SRCS)
ASM_SRCS += $(QP_ASMS)

BIN_DIR := build_$(TARGET)

ASFLAGS = -g $(ARM_CPU) $(ARM_FPU) $(ASM_CPU) $(ASM_FPU)

CFLAGS = -c -g $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) -mthumb -Wall \
	-DHIL_TEST -DUNITY_INCLUDE_CONFIG_H\
	-ffunction-sections -fdata-sections \
	-O $(INCLUDES) $(DEFINES) \
	-DCMOCK_MEM_STATIC -DCMOCK_MEM_SIZE=1024 -DCMOCK_MEM_ALIGN=2

CPPFLAGS = -c -g $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) -mthumb -Wall \
	-ffunction-sections -fdata-sections -fno-rtti -fno-exceptions \
	-O $(INCLUDES) $(DEFINES)

LINKFLAGS = -T$(LD_SCRIPT) $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) -mthumb \
	-specs=nosys.specs -specs=nano.specs \
	-Wl,-Map,$(BIN_DIR)/$(OUTPUT).map,--cref,--gc-sections $(LIB_DIRS)

ASM_OBJS     := $(patsubst %.s,%.o,  $(notdir $(ASM_SRCS)))
C_OBJS       := $(patsubst %.c,%.o,  $(notdir $(C_SRCS)))
CPP_OBJS     := $(patsubst %.cpp,%.o,$(notdir $(CPP_SRCS)))

TARGET_BIN   := $(BIN_DIR)/$(OUTPUT).bin
TARGET_ELF   := $(BIN_DIR)/$(OUTPUT).elf
ASM_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(ASM_OBJS))
C_OBJS_EXT   := $(addprefix $(BIN_DIR)/, $(C_OBJS))
C_DEPS_EXT   := $(patsubst %.o, %.d, $(C_OBJS_EXT))
CPP_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(CPP_OBJS))
CPP_DEPS_EXT := $(patsubst %.o, %.d, $(CPP_OBJS_EXT))

# create $(BIN_DIR) if it does not exist
ifeq ("$(wildcard $(BIN_DIR))","")
$(shell $(MKDIR) $(BIN_DIR))
endif

#-----------------------------------------------------------------------------
# rules
#
# Include dependency files if they exist
# -include $(C_DEPS_EXT) $(CPP_DEPS_EXT)

.PHONY : run norun flash

ifeq ($(MAKECMDGOALS),norun)
all : $(TARGET_BIN)
norun : all
else
all : $(TARGET_BIN) run
endif

$(TARGET_BIN): $(TARGET_ELF)
	$(BIN) -O binary $< $@
	$(FLASH) $@
	@echo
	@echo Reset the board MANUALLY!

$(TARGET_ELF) : $(ASM_OBJS_EXT) $(C_OBJS_EXT) $(CPP_OBJS_EXT)
	$(LINK) $(LINKFLAGS) -o $@ $^ $(LIBS)

flash :
	$(FLASH) $(TARGET_BIN)
	@echo
	@echo Reset the board MANUALLY!

$(BIN_DIR)/%.d : %.c
	$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $< > $@

$(BIN_DIR)/%.d : %.cpp
	$(CPP) -MM -MT $(@:.d=.o) $(CPPFLAGS) $< > $@

$(BIN_DIR)/%.o : %.s
	$(AS) $(ASFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

.PHONY : clean show

# include dependency files only if our goal depends on their existence
ifneq ($(MAKECMDGOALS),clean)
  ifneq ($(MAKECMDGOALS),show)
-include $(C_DEPS_EXT) $(CPP_DEPS_EXT)
  endif
endif


clean :
	-$(RM) $(BIN_DIR)/*.o \
	$(BIN_DIR)/*.d \
	$(BIN_DIR)/*.bin \
	$(BIN_DIR)/*.elf \
	$(BIN_DIR)/*.map
	
show:
	@echo PROJECT = $(PROJECT)
	@echo CONF = $(CONF)
	@echo DEFINES = $(DEFINES)
	@echo ASM_FPU = $(ASM_FPU)
	@echo ASM_SRCS = $(ASM_SRCS)
	@echo C_SRCS = $(C_SRCS)
	@echo CPP_SRCS = $(CPP_SRCS)
	@echo ASM_OBJS_EXT = $(ASM_OBJS_EXT)
	@echo C_OBJS_EXT = $(C_OBJS_EXT)
	@echo C_DEPS_EXT = $(C_DEPS_EXT)
	@echo CPP_DEPS_EXT = $(CPP_DEPS_EXT)
	@echo TARGET_ELF = $(TARGET_ELF)
