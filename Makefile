TARGET_EXEC ?= plato.prg

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

RSCFILE = $(SRC_DIRS)/PLATO.RSC

CC=m68k-atari-mint-gcc-8
AS=m68k-atari-mint-as

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS=-m68000 -O0 
LDFLAGS=-lm -lgem -lcmini -nostdlib -lgcc -lcmini

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) ../libcmini/build/startup.o $(OBJS) -o $@ -L../libcmini/build $(LDFLAGS)
	cp $(RSCFILE) $(BUILD_DIR)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) -I ../libcmini/include $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
