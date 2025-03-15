all:
#	compile object
	gcc -c -o Hello.o Hello.c -I ./
	gcc -c -o main.o main.c -I ./
# Linking make file
	gcc -o a.out main.o Hello.o

stage1:
	gcc -E main.c -o main.i

stage2:
	gcc -S main.i -o main.S

stage3:
	gcc -c main.S -o main.o

stage4:
	gcc -o main.out main.o

clean:
	rm -rf *.out *.i *.S *.o
