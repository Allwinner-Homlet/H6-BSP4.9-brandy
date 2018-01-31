

objects = crc.o script.o mbr2env.o


edit:$(objects)
	gcc -o mbr2env $(objects) -static
	rm -rf $(objects)


clean:
	rm -rf update_mbr $(objects)
