PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR    := $(abspath $(CURDIR))
BUILD_SUB    ?= linux

ifeq ($(abspath $(PROJECT_ROOT)),$(BUILD_DIR))
	CREATE_BUILD_DIR := yes
else
	CREATE_BUILD_DIR := no
endif

ifeq ($(BUILD_MODE),debug)
	ifeq ($(CREATE_BUILD_DIR),yes)
		BUILD_DIR := $(BUILD_DIR)/build/debug.$(BUILD_SUB)/
	endif
	CFLAGS += -g -O0
else
	ifeq ($(CREATE_BUILD_DIR),yes)
		BUILD_DIR := $(BUILD_DIR)/build/release.$(BUILD_SUB)/
	endif
	CFLAGS += -O2
endif

TARGET        := $(BUILD_DIR)/vbtests
BUILD_OBJ_DIR := $(BUILD_DIR)/obj/

SRC_APP  := $(wildcard $(PROJECT_ROOT)src/app/*.c)
SRC_UTL  := $(wildcard $(PROJECT_ROOT)src/utl/*.c)
SRC_TST  := $(wildcard $(PROJECT_ROOT)tests/*.c)

OBJS_APP := $(patsubst $(PROJECT_ROOT)src/app/%.c, $(BUILD_OBJ_DIR)%.o, $(SRC_APP))
OBJS_UTL := $(patsubst $(PROJECT_ROOT)src/utl/%.c, $(BUILD_OBJ_DIR)%.o, $(SRC_UTL))
OBJS_TST := $(patsubst $(PROJECT_ROOT)tests/%.c,   $(BUILD_OBJ_DIR)%.o, $(SRC_TST))

# needed for dependency tracking
CFLAGS += -MMD -MP

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS_APP) $(OBJS_UTL) $(OBJS_TST)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJS_APP): $(BUILD_OBJ_DIR)%.o: $(PROJECT_ROOT)src/app/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(PROJECT_ROOT)include -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(OBJS_UTL): $(BUILD_OBJ_DIR)%.o: $(PROJECT_ROOT)src/utl/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(PROJECT_ROOT)include -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(OBJS_TST): $(BUILD_OBJ_DIR)%.o: $(PROJECT_ROOT)tests/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(PROJECT_ROOT)include -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

-include $(OBJS_APP:.o=.d) $(OBJS_UTL:.o=.d) $(OBJS_TST:.o=.d)

clean:
	rm -rf $(BUILD_DIR)
