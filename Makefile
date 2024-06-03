CC = gcc
INCLUDES = -Iinclude
LIBS =

C_FLAGS ?= 
C_FLAGS += -g -Wall -Wextra -pedantic -Wno-incompatible-pointer-types -std=c17 -Wno-unused-command-line-argument
C_FLAGS += $(INCLUDES)
C_FLAGS += $(LIBS)

build_dir := bin
obj_dir := obj

all: test benchmark

bin:
	@mkdir -p bin

obj:
	@mkdir -p obj

test: src/union2by2_test.c $(obj_dir)/utils.o | bin
	$(CC) $(C_FLAGS) -o $(build_dir)/test-u2b2 $^

$(obj_dir)/utils.o: src/utils.c include/utils.h | obj
	$(CC) -c $(C_FLAGS) -o $@ $<

$(obj_dir)/union2by2.o: src/union2by2.c include/union2by2.h src/avx_utils.c src/neon_utils.c src/shuffle_mat.c | obj
	$(CC) -c $(C_FLAGS) -o $@ $<

benchmark: src/union2by2_benchmark.c obj/union2by2.o obj/utils.o | bin
	$(CC) $(C_FLAGS) -o $(build_dir)/benchmark-u2b2 $^

clean:
	rm -rf $(build_dir)
