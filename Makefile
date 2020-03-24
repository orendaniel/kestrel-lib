all:
	gcc src/*.c -I /usr/include/lua5.3/ -llua5.3 -lv4l2 -fPIC -shared -o /usr/local/lib/lua/5.3/kestrel.so

clean:
	rm /usr/local/share/lua/5.3/kestrel.so
