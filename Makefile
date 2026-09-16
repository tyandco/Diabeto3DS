ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. Install devkitPro and the 3DS toolchain first.")
endif

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. Install devkitARM from devkitPro first.")
endif

include $(DEVKITARM)/3ds_rules

TARGET      := Diabeto3DS
BUILD       := build
SOURCES     := source
INCLUDES    := include
ROMFS       := romfs

APP_TITLE   := Diabeto
APP_DESC    := Offline diabetes prevention companion
APP_AUTHOR  := tyandco
UNIQUE_ID   := 0xD1A8E7

export ARCH     := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
export CFLAGS   = -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH) $(INCLUDE) -D__3DS__
export CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
export ASFLAGS  := -g $(ARCH)
export LDFLAGS  := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
export LIBS     := -lctru -lm
LIBDIRS         := $(CTRULIB)

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES        := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

ifeq ($(strip $(CPPFILES)),)
export LD       := $(CC)
else
export LD       := $(CXX)
endif

export OFILES   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh
export SMDHFLAGS  := --create "$(APP_TITLE)" "$(APP_DESC)" "$(APP_AUTHOR)" $(CTRULIB)/default_icon.png $(CURDIR)/$(TARGET).smdh

ifneq ($(strip $(ROMFS)),)
export _3DSXFLAGS += --romfs=$(CURDIR)/$(ROMFS)
endif

.PHONY: all clean cia

all: $(BUILD) $(OUTPUT).3dsx

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(OUTPUT).3dsx: $(OUTPUT).elf $(OUTPUT).smdh

$(OUTPUT).smdh:
	@smdhtool $(SMDHFLAGS)

cia: $(OUTPUT).elf $(OUTPUT).smdh
	@echo "CIA packaging is prepared but needs final icon/banner assets."
	@echo "After assets arrive, add meta/banner.bnr and run makerom with meta/app.rsf."

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(TARGET).elf $(TARGET).smdh $(TARGET).cia $(TARGET).map

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
