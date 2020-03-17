#include <lua.h>
#include <lauxlib.h>
#include <luaconf.h>
#include <lualib.h>
#include "image.h"
#include "device.h"
#include "contour.h"

#define IMAGE_MT	"kestrel-image"
#define DEVICE_MT 	"kestrel-device"
#define CONTOUR_MT 	"kestrel-contour"

static int lua_new_image(lua_State* L) {
	size_t c = luaL_checkinteger(L, 1);
	size_t w = luaL_checkinteger(L, 2);
	size_t h = luaL_checkinteger(L, 3);
	
	Image* img = new_image(c, w, h);

	if (img == NULL)
		return luaL_error(L, "image allocation error");

	Image** pimg = (Image**)lua_newuserdata(L, sizeof(Image*));

	*pimg = img;

	luaL_getmetatable(L, IMAGE_MT);
	lua_setmetatable(L, -2);

	return 1;	
}

static int lua_read_rgb_pixel_map(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	
	Image* img = read_rgb_pixel_map(name);

	if (img == NULL)
		return luaL_error(L, "image allocation error");

	Image** pimg = (Image**)lua_newuserdata(L, sizeof(Image*));
	*pimg = img;

	luaL_getmetatable(L, IMAGE_MT);
	lua_setmetatable(L, -2);

	return 1;	
}


static int lua_set_at(lua_State* L) {
	Image** pimg 	= luaL_checkudata(L, 1, IMAGE_MT);
	size_t 	c 		= luaL_checkinteger(L, 2) -1;
	size_t 	x 		= luaL_checkinteger(L, 3) -1;
	size_t 	y 		= luaL_checkinteger(L, 4) -1;
	size_t 	v 		= luaL_checkinteger(L, 5);
	set_at(*pimg, c, x, y, v);

	
	return 0;
}

static int lua_get_at(lua_State* L) {
	Image** pimg 	= luaL_checkudata(L, 1, IMAGE_MT);
	size_t 	c 		= luaL_checkinteger(L, 2) -1;
	size_t 	x 		= luaL_checkinteger(L, 3) -1;
	size_t 	y 		= luaL_checkinteger(L, 4) -1;
	size_t 	def_v 	= luaL_optinteger(L, 5, 0);

	lua_pushinteger(L, get_at(*pimg, c, x, y, def_v));

	return 1;
}

static int lua_write_rgb_pixel_map(lua_State* L) {
	Image** pimg 		= luaL_checkudata(L, 1, IMAGE_MT);
	const char* name 	= luaL_checkstring(L, 2);
	write_rgb_pixel_map(name, *pimg);
	
	return 0;
}

static int lua_in_range(lua_State* L) {
	Image** pimg	= luaL_checkudata(L, 1, IMAGE_MT);

	luaL_checktype(L, 2, LUA_TTABLE);
	luaL_checktype(L, 3, LUA_TTABLE);

	value_t on_value 	= luaL_optinteger(L, 4, 255);
	value_t off_value 	= luaL_optinteger(L, 5, 0);

	size_t n_lower = luaL_len(L, 2);  // size of lower table
	size_t n_upper = luaL_len(L, 3);  // size of upper
	

	if (n_lower == n_upper && n_lower == (*pimg)->channels) {
		value_t* lowers = calloc(n_lower, sizeof(value_t));
		value_t* uppers = calloc(n_upper, sizeof(value_t));

		if (lowers == NULL || uppers == NULL)
			return 0;

		for (int i = 1; i <= n_lower; i++) {
			value_t top;
			lua_rawgeti(L, 2, i);
			top = lua_gettop(L);
			lowers[i-1] = top;

			lua_rawgeti(L, 3, i);
			top = lua_gettop(L);
			uppers[i-1] = top;

		}
		
		Image** prange = (Image**)lua_newuserdata(L, sizeof(Image*));
		*prange = in_range(*pimg, lowers, uppers, on_value, off_value);
	
		luaL_getmetatable(L, IMAGE_MT);
		lua_setmetatable(L, -2);
		return 1;
	
	}
	else
		return 0;
		
}

static int lua_rgb_to_hsv(lua_State* L) {
	Image** pimg 	= luaL_checkudata(L, 1, IMAGE_MT);
	Image* 	hsv 	= rgb_to_hsv(*pimg);
	Image** phsv 	= (Image**)lua_newuserdata(L, sizeof(Image*));

	*phsv = hsv;

	luaL_getmetatable(L, IMAGE_MT);
	lua_setmetatable(L, -2);
	
	return 1;
}

static int lua_gc_image(lua_State* L) {
	Image** pimg = (Image**)luaL_checkudata(L, 1, IMAGE_MT);
	free_image(*pimg);
	return 0;
}

static int lua_open_device(lua_State* L) {
	
	const char* path 	= luaL_checkstring(L, 1);
	size_t 		width 	= luaL_optinteger(L, 2, 160);
	size_t 		height	= luaL_optinteger(L, 3, 120);
	Device* 	dev 	= new_device(path, width, height);

	if (dev == NULL)
		return luaL_error(L, "error opening device");
	if (dev->fd < 0)
		return luaL_error(L, "device in use");
		
	Device** pdev = (Device**)lua_newuserdata(L, sizeof(Device*));

	*pdev = dev;

	luaL_getmetatable(L, DEVICE_MT);
	lua_setmetatable(L, -2);

	return 1;
}


static int lua_read_frame(lua_State* L) {
	
	Device** pdev = (Device**)luaL_checkudata(L, 1, DEVICE_MT);
	Image* img = read_frame(*pdev);

	Image** pimg = (Image**)lua_newuserdata(L, sizeof(Image*));
	*pimg = img;

	luaL_getmetatable(L, IMAGE_MT);
	lua_setmetatable(L, -2);

	return 1;
}

static int lua_close_device(lua_State* L) {
	Device** pdev = (Device**)luaL_checkudata(L, 1, DEVICE_MT);
	free_device(*pdev);
	return 0;
}

static int lua_find_contours(lua_State* L) {
	return 0;
}

static int lua_read_frame(lua_State* L){

	return 0;
}

int LUA_API luaopen_kestrel(lua_State* L) {
	const luaL_Reg lib[] = {
		{"newimage",			lua_new_image},
		{"read_rgb_pixelmap",	lua_read_rgb_pixel_map},
		{"opendevice",			lua_open_device},
		{"findcontours",		lua_find_contours},
		{NULL, NULL},
	};

	if (luaL_newmetatable(L, IMAGE_MT)) {
		const luaL_Reg image_funcs[] = {
				{"getat", 				lua_get_at},
				{"setat", 				lua_set_at},
				{"write_rgb_pixelmap", 	lua_write_rgb_pixel_map},
				{"inrange", 			lua_in_range},
				{"rgb_to_hsv", 			lua_rgb_to_hsv},
				{"__gc", 				lua_gc_image},
				{NULL, NULL},
			};
		luaL_setfuncs(L, image_funcs, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
	lua_pop(L, 1); // discard metatable
	
	if (luaL_newmetatable(L, DEVICE_MT)) {
		const luaL_Reg device_funcs[] = {
				{"readframe", 	lua_read_frame},
				{"close", 		lua_close_device},
				{"__gc", 		lua_close_device},
				{NULL, NULL},
			};
		luaL_setfuncs(L, device_funcs, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
	lua_pop(L, 1); // discard metatable

	if (luaL_newmetatable(L, CONTOUR_MT)) {
		const luaL_Reg contour_funcs[] = {
				{"center",	lua_read_frame},
				{NULL, NULL},
			};
		luaL_setfuncs(L, contour_funcs, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
	lua_pop(L, 1); // discard metatable

	lua_createtable(L, sizeof(lib) / sizeof(lib[0]), 0);
	luaL_setfuncs(L, lib, 0);
	return 1;
}
