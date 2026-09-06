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

SRC_APP     := $(wildcard $(PROJECT_ROOT)src/app/*.c)
SRC_UTL     := $(wildcard $(PROJECT_ROOT)src/utl/*.c)

OBJS_APP      := $(patsubst $(PROJECT_ROOT)src/app/%.c,$(BUILD_OBJ_DIR)%.o,$(SRC_APP))
OBJS_UTL      := $(patsubst $(PROJECT_ROOT)src/utl/%.c,$(BUILD_OBJ_DIR)%.o,$(SRC_UTL))

STATIC_LIB = $(BUILD_DIR)/libvb.a
SHARED_LIB = $(BUILD_DIR)/libvb.so

# shared library
CFLAGS += -fPIC
# dependency tracking
CFLAGS += -MMD -MP

.PHONY: all static shared clean

all: static shared

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

$(STATIC_LIB): $(OBJS_APP) $(OBJS_UTL)
	ar rcs $@ $^

$(SHARED_LIB): $(OBJS_APP)  $(OBJS_UTL)
	$(CC) -shared $(LDFLAGS) -o $@ $^

$(OBJS_APP): $(BUILD_OBJ_DIR)%.o: $(PROJECT_ROOT)src/app/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(PROJECT_ROOT)include -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(OBJS_UTL): $(BUILD_OBJ_DIR)%.o: $(PROJECT_ROOT)src/utl/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(PROJECT_ROOT)include -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

-include $(OBJS_APP:.o=.d) $(OBJS_UTL:.o=.d)

clean:
	rm -rf $(BUILD_DIR)
