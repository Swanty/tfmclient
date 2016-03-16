CC = gcc
SRC_DIR = src
RESOURCE_DIR = res
INCLUDE_DIR = npapi

CFLAGS := -Wall -O2 -fomit-frame-pointer -march=native -pipe -DXP_UNIX=1 -DMOZ_X11=1 -fPIC -std=gnu99  -D_GNU_SOURCE -I$(INCLUDE_DIR) `pkg-config --cflags --libs gtk+-2.0`

BUILD_DIR = .
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/

EXECUTABLE := $(BIN_DIR)/transformice

LDFLAGS = -ldl -lX11

CFILES := $(wildcard $(SRC_DIR)/*.c)

make_path = $(addsuffix $(1), $(basename $(subst $(2), $(3), $(4))))
src_to_obj = $(call make_path,.o, $(SRC_DIR), $(OBJ_DIR), $(1))

OBJECTS := $(foreach src, $(CFILES), $(call src_to_obj, $(src)))

.DEFAULT_GOAL := all

Debug: all

all: $(EXECUTABLE)

clean:
	rm -rvf $(OBJ_DIR)/* $(EXECUTABLE)

Makefile: ;

$(EXECUTABLE): $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) -Wl,--format=binary -Wl,$(RESOURCE_DIR)/TransformiceChargeur.swf -Wl,--format=default -m64

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

