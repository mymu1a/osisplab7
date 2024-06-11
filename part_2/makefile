build: generate file_modify

file_modify: main.o gengetopt/cmdline.o
	gcc -o file_modify $^

generate: generate.o gengetopt/generate_cmdline.o
	gcc -o generate $^
