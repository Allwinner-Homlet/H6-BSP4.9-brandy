
objects = programmer.o part.o file.o script.o crc.o

edit:$(objects)
	gcc -o programmer_img $(objects) -static
	rm -rf $(objeects)
	cp programmer_img ../../../tools/pack/pctools/linux/mod_update/
clean:
	rm -rf programmer_img $(objects)
