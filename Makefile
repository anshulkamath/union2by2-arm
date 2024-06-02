CC = gcc
INCLUDES = -Iinclude
LIBS =

C_FLAGS ?= 
C_FLAGS += -g -Wall -Wextra -pedantic -Wno-incompatible-pointer-types -std=c17 -Wno-unused-command-line-argument
C_FLAGS += $(INCLUDES)
C_FLAGS += $(LIBS)

build_dir := bin

all:

bin:
	@mkdir -p bin

test: src/union2by2_test.c src/union2by2.c | bin
	$(CC) $(C_FLAGS) -o $(build_dir)/test-u2b2 $<

clean:
	rm -rf $(build_dir)
