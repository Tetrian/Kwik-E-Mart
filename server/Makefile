# Global variables
PROJECT_NAME := $(shell basename "$(PWD)" | tr '[:upper:]' '[:lower:]')

# Compiler settings
CC=gcc
CCVER=--std=c17
CFLAGS=-Wno-implicit-fallthrough -Wmaybe-uninitialized -Wno-missing-field-initializers -Wcast-align -Wpedantic -Wnull-dereference \
				-Wformat=2 -Wformat-truncation -Wformat-overflow -Wundef -Wmisleading-indentation \
				-Wall -Wextra -Wunused -Wshadow -Wwrite-strings -Wdouble-promotion \
				-Wduplicated-cond -Wduplicated-branches -Wlogical-op -ffunction-sections -pthread -fno-strict-aliasing \
				-g -ggdb3
PG_FLAGS=$(shell pkg-config --cflags libpq) -lpq
POSIX=-D_POSIX_C_SOURCE=200809L

# Debug version by default
FLDEBUG=-DDEBUG -g -O0
FLRELEASE=-DNDEBUG -O2
FLSHUTUP=-DSHUTUP
DEBUG ?= 1

ifeq ($(DEBUG), 1)
    CFLAGS+=$(FLDEBUG)
else
    CFLAGS+=$(FLRELEASE)
endif

ifeq ($(SHUTUP), 1)
    CFLAGS+=$(FLSHUTUP)
endif

# Flags
CFLAGS+=$(POSIX)

# Directories
OBJDIR=./.obj
OBJDIR_CORE=$(OBJDIR)/core
OBJDIR_HELPER=$(OBJDIR)/helper
OBJDIR_SERVER=$(OBJDIR)
BINDIR=./bin
CORE=./src/core
HELPER=./src/helper
SERVER=./src


# Retrieve files from obj
SRC_CORE=$(wildcard $(CORE)/*.c)
OBJ_CORE=$(patsubst $(CORE)/%.c,$(OBJDIR)/%.o,$(SRC_CORE))
DEP_CORE=$(OBJ_CORE:%.o=%.d)

SRC_HELPER=$(wildcard $(HELPER)/*.c)
OBJ_HELPER=$(patsubst $(HELPER)/%.c,$(OBJDIR)/%.o,$(SRC_HELPER))
DEP_HELPER=$(OBJ_HELPER:%.o=%.d)

SRC_SERVER=$(wildcard $(SERVER)/*.c)
OBJ_SERVER=$(patsubst $(SERVER)/%.c,$(OBJDIR_SERVER)/%.o,$(SRC_SERVER))
DEP_SERVER=$(OBJ_SERVER:%.o=%.d)

.DEFAULT_GOAL := help

.PHONY: help
help: Makefile
	@echo " Choose a command to run in "$(PROJECT_NAME)":"
	@echo
	@sed -n 's/^##//p' $< | column -t -s ':' | sed -e 's/^/ /'
	@echo

# common
$(OBJDIR)/%.o: $(CORE)/%.c | $(OBJDIR)
	$(CC) $(CCVER) $(CFLAGS) -MMD -c $< -o $@ $(PG_FLAGS)

$(OBJDIR)/%.o: $(HELPER)/%.c | $(OBJDIR)
	$(CC) $(CCVER) $(CFLAGS) -MMD -c $< -o $@

$(OBJDIR_SERVER)/%.o: $(SERVER)/%.c | $(OBJDIR_SERVER)
	$(CC) $(CCVER) $(CFLAGS) -MMD -c $< -o $@ $(PG_FLAGS) 

## server: Generate server executable
server: $(OBJ_SERVER) $(OBJ_MISC) $(OBJ_CORE) $(OBJ_HELPER) | $(BINDIR)
	$(CC) $(CCVER) $(CFLAGS) $^ -o $(BINDIR)/server $(PG_FLAGS)


# Make directory if does not exist
$(BINDIR) $(OBJDIR) $(OBJDIR_CLIENT) $(OBJDIR_SERVER):
	mkdir -p $@

-include $(DEP_MISC)
-include $(DEP_SERVER)

## clean: Cleanup object files and executable
clean:
	$(info --> Cleaning up project)
	rm -rf $(OBJDIR)/* $(BINDIR)/*
