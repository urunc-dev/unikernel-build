default: hello

hello.o: hello.c 
	gcc -c hello.c -o hello.o 

hello: hello.o 
	gcc hello.o -o hello 

hello-rumprun: hello.c
	${CC} hello.c -o hello-rumprun

hello.spt: hello-rumprun
	rumprun-bake solo5_spt hello.spt hello-rumprun

hello.hvt: hello-rumprun
	rumprun-bake solo5_hvt hello.hvt hello-rumprun

clean:
	-rm -f hello.o 
	-rm -f hello
	-rm -f hello-rumprun
	-rm -f hello.spt
	-rm -f hello.hvt
