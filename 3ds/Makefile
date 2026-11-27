TARGET      := A-brick-cube-3ds
BUILD       := build
SOURCES     := source
INCLUDES    := source ../src
ROMFS       := ../data

APP_TITLE   := A Brick Cube 3DS
APP_AUTHOR  := aamiso
APP_DESC    := Port 3DS

PREFIX      := arm-none-eabi-
CC          := $(PREFIX)gcc
CXX         := $(PREFIX)g++
AS          := $(PREFIX)as
AR          := $(PREFIX)ar
LD          := $(PREFIX)g++
SMDHTOOL    := smdhtool

ARCH        := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS      := -g -Wall -O2 -mword-relocations -fomit-frame-pointer -ffunction-sections $(ARCH)
CFLAGS      += $(INCLUDE) -D__3DS__
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS     := -g $(ARCH)
LDFLAGS     := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS        := -lcitro2d -lcitro3d -lctru -lm

LIBDIRS     := $(CTRULIB)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export _3DSXFLAGS += --romfs=$(CURDIR)/$(ROMFS)

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $(OUTPUT).3dsx

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(TARGET).elf $(TARGET).smdh icon.png

else

DEPENDS := $(OFILES:.o=.d)

icon.png :
	@python3 -c "import struct, zlib; w=h=48; raw=b''.join(b'\x00'+b'\x30\x70\xc0\xff'*w for _ in range(h)); png=b'\x89PNG\r\n\x1a\n'+b''.join(struct.pack('>I',len(d))+t+d+struct.pack('>I',zlib.crc32(t+d)&0xffffffff) for t,d in [(b'IHDR',struct.pack('>IIBBBBB',w,h,8,6,0,0,0)),(b'IDAT',zlib.compress(raw)),(b'IEND',b'')]); open('$@','wb').write(png)"

$(OUTPUT).smdh : icon.png
	@$(SMDHTOOL) --create "$(APP_TITLE)" "$(APP_DESC)" "$(APP_AUTHOR)" $< $@

$(OUTPUT).3dsx : $(OUTPUT).elf $(OUTPUT).smdh
	@3dsxtool $< $@ --smdh=$(OUTPUT).smdh $(_3DSXFLAGS)
	@echo "Compilado: $(notdir $@)"

$(OUTPUT).elf : $(OFILES)
	@$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

%.o : %.c
	@echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

%.o : %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

%.o : %.s
	@echo $(notdir $<)
	@$(AS) $(ASFLAGS) $< -o $@

-include $(DEPENDS)

endif
