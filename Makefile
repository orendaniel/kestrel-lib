LUA_VERSION=5.4

all:
	mkdir -p /usr/local/lib/lua/$(LUA_VERSION)
	gcc src/*.c -I /usr/include/lua$(LUA_VERSION)/ -llua$(LUA_VERSION) -lv4l2 -lm -fPIC -shared -o /usr/local/lib/lua/$(LUA_VERSION)/kestrel.so

clean:
	rm /usr/local/share/lua/$(LUA_VERSION)/kestrel.so
