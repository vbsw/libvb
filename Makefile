PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR    := $(abspath $(CURDIR))

ifeq ($(abspath $(PROJECT_ROOT)),$(BUILD_DIR))
	CREATE_BUILD_DIR := yes
else
	CREATE_BUILD_DIR := no
endif

ifeq ($(BUILD_MODE),debug)
	ifeq ($(CREATE_BUILD_DIR),yes)
		BUILD_DIR := $(BUILD_DIR)/build/debug.linux/
	endif
	CFLAGS += -g -O0
else
	ifeq ($(CREATE_BUILD_DIR),yes)
		BUILD_DIR := $(BUILD_DIR)/build/release.linux/
	endif
	CFLAGS += -O2
endif

BUILD_OBJ_DIR := $(BUILD_DIR)/obj/

LIB_SOURCES   := $(wildcard $(PROJECT_ROOT)*.c)
LIB_OBJS      := $(patsubst $(PROJECT_ROOT)%.c,$(BUILD_OBJ_DIR)%.o,$(LIB_SOURCES))

STATIC_LIB = $(BUILD_DIR)/libvb.a
SHARED_LIB = $(BUILD_DIR)/libvb.so

# needed for shared library
CFLAGS += -fPIC
# needed for dependency tracking
CFLAGS += -MMD -MP

.PHONY: all static shared clean

all: static shared

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

$(STATIC_LIB): $(LIB_OBJS)
	ar rcs $@ $^

$(SHARED_LIB): $(LIB_OBJS)
	$(CC) -shared $(LDFLAGS) -o $@ $^

$(BUILD_OBJ_DIR)%.o: $(PROJECT_ROOT)%.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

-include $(LIB_OBJS:.o=.d)

clean:
	rm -rf $(BUILD_DIR)
