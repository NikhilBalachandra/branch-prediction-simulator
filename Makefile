# Disable built-in rules and variables
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

MAKEFILENAME := $(firstword $(MAKEFILE_LIST)) # Typically Makefile or GNUmakefile

# To use a different compiler, set the CC variable.
CC := cc
OPT = -O3
WARN = -Wall -Winline
CFLAGS = $(OPT) $(WARN)
LDFLAGS = -lm
SOURCE_DIR := src
OBJECT_DIR := obj/release
EXE_NAME := sim

# Run with Make DEBUG=1 to enable debug mode
# For debug builds do the following
# 1) Add -g flag and define DEBUG=1 macro.
# 2) Remove $(OPT) flags.
DEBUG ?= 0
ifeq ($(DEBUG), 1)
DEBUG_SYMBOLS ?= 1
CFLAGS += -DDEBUG
CFLAGS := $(filter-out $(OPT),$(CFLAGS))
OBJECT_DIR = obj/debug
endif

DEBUG_SYMBOLS ?= 0
ifeq ($(DEBUG_SYMBOLS), 1)
	CFLAGS += -g
endif

# Find all source .c files.
SOURCES = $(wildcard $(SOURCE_DIR)/*.c)
HEADERS = $(wildcard $(SOURCE_DIR)/*.h)

# List corresponding compiled object files here (.o files)
OBJECTS = $(patsubst src/%.c,$(OBJECT_DIR)/%.o,$(SOURCES))

# Load dependency rules if they exist. These files are automatically generated
# by the gcc compiler when -MMD flag is used.
DEPENDS := $(patsubst %.o,%.d,$(OBJECTS))
-include $(DEPENDS)

# Report files
REPORT_FILES = $(wildcard reports/*)

###############################################################################

# default rule
.DEFAULT_GOAL := all
.PHONY: all
all: $(EXE_NAME)
	@echo "Done building sim binary"

# Create object directory if it doesn't exist.
$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

# generic rule for converting any .c file to .o file
# Depends on $(OBJECT_DIR) to ensure that the object directory exists.
$(OBJECTS): $(OBJECT_DIR)/%.o : $(SOURCE_DIR)/%.c | $(OBJECT_DIR)
	$(CC) $(CFLAGS) -MMD -MP -MF $(OBJECT_DIR)/$*.d -c -o $@ $<

# Generate compile_commands.json file for use with clang tools
# Compile commands depends on build flags and source code files.
# clean build is required for the bear to inspect build commands.
compile_commands.json: $(MAKEFILENAME) $(SOURCES) $(HEADERS)
	bear -- $(MAKE) clean build

# type "make clobber" to remove all .o files (leaves sim binary)
.PHONY: clobber
clobber:
	rm -f $(OBJECT_DIR)/*.o
	rm -f $(OBJECT_DIR)/*.d
	rm -f $(REPORT_FILES)

# type "make clean" to remove all .o files plus the sim binary
.PHONY: clean
clean: clobber
	rm -f $(EXE_NAME)

# rule for building sim binary.
# Note that assignment requires sim binary to be generated in root directory.
$(EXE_NAME): $(OBJECTS)
	$(CC) -o $(EXE_NAME) $(OBJECTS) $(LDFLAGS)

# Tests copy binary to tests folder as test output files depends on the path
# of the binary and that of the trace file.
.PHONY: tests
tests: $(EXE_NAME)
	cp $(EXE_NAME) tests/
	cd tests; ./sim smith 3 gcc_trace.txt | diff -iw ./val_smith_1.txt -
	cd tests; ./sim smith 1 jpeg_trace.txt | diff -iw ./val_smith_2.txt -
	cd tests; ./sim smith 4 perl_trace.txt | diff -iw ./val_smith_3.txt -
	cd tests; ./sim bimodal 6 gcc_trace.txt | diff -iw ./val_bimodal_1.txt -
	cd tests; ./sim bimodal 12 gcc_trace.txt | diff -iw ./val_bimodal_2.txt -
	cd tests; ./sim bimodal 4 jpeg_trace.txt | diff -iw ./val_bimodal_3.txt -
	cd tests; ./sim gshare 9 3 gcc_trace.txt | diff -iw ./val_gshare_1.txt -
	cd tests; ./sim gshare 14 8 gcc_trace.txt | diff -iw ./val_gshare_2.txt -
	cd tests; ./sim gshare 11 5 jpeg_trace.txt | diff -iw ./val_gshare_3.txt -
	cd tests; ./sim hybrid 8 14 10 5 gcc_trace.txt | diff -iw ./val_hybrid_1.txt -
	rm tests/$(EXE_NAME)

.PHONY: misprediction_rate_report.sh
misprediction_rate_report.sh: $(EXE_NAME)
	./misprediction_rate_report.sh

project2.zip: $(SOURCES) $(HEADERS) Makefile report.pdf
	zip $@ $(SOURCES) $(HEADERS) Makefile report.pdf
