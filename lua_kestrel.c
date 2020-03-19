#include <lua.h>
#include <lauxlib.h>
#include <luaconf.h>
#include <lualib.h>

#define IMAGE_MT	"kestrel-image"
#define DEVICE_MT 	"kestrel-device"
#define CONTOUR_MT 	"kestrel-contour"

#include "common.h"
#include "image.h"
#include "device.h"
#include "contour.h"

//HELPERS
//-------------------------------------------------------------------------------
static int get_by_index(lua_State* L, int tindex, int i) {
	lua_pushinteger(L, i);
	lua_gettable(L, tindex);
}

static int lua_arth_image(lua_State* L, int (*fn)(value_t current, float x)) {
	Image** pimg 	= (Image**)luaL_checkudata(L, 1, IMAGE_MT);
	Image* 	new_img =  new_image((*pimg)->channels, (*pimg)->width, (*pimg)->height);
	if (new_img == NULL)
		return luaL_error(L, "image allocation error");

	float x = luaL_checknumber(L, 2);


	for (int i = 0; i < (*pimg)->height; i++) {
		for (int j = 0; j < (*pimg)->width; j++) {
			for (int c = 0; c < (*pimg)->channels; c++){
				value_t v = get_at(*pimg, c, j, i, 0);
				int nv = (*fn)(v, x);
				if (nv < 0) {
					nv = 0;
				}
				else if (nv > MAX_VALUE) {
					nv = MAX_VALUE;
				}
				set_at(new_img, c, j, i, (value_t)nv);
			}
		}
	}

	Image** pnew_img = (Image**)lua_newuserdata(L, sizeof(Image*));
	*pnew_img = new_img;

	luaL_getmetatable(L, IMAGE_MT);
	lua_setmetatable(L, -2);

}
//-------------------------------------------------------------------------------

//KESTREL
//-------------------------------------------------------------------------------
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

	Image** pimg = (Image**)lua_newuserdata(L, sizeof(Image*));
	*pimg = img;

	luaL_getmetatable(L, IMAGE_MT);
	lua_setmetatable(L, -2);

	return 1;	
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

static int lua_open_device(lua_State* L) {
	const char* path 	= luaL_checkstring(L, 1);
	size_t 		width 	= luaL_optinteger(L, 2, DEFAULT_DEVICE_WIDTH);
	size_t 		height	= luaL_optinteger(L, 3, DEFAULT_DEVICE_HEIGHT);
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

static int lua_find_contours(lua_State* L) {
	Image** pimg 	= (Image**)luaL_checkudata(L, 1, IMAGE_MT);
	size_t 	steps_x = luaL_optinteger(L, 2, DEFAULT_STEPS_TRACING);
	size_t 	steps_y = luaL_optinteger(L, 2, DEFAULT_STEPS_TRACING);

	size_t contours_amount;
	Contour** cnts = find_contours(*pimg, &contours_amount, steps_x, steps_y);

	lua_createtable(L, contours_amount, 0);
	for (int i = 0; i < contours_amount; i++){
		
		lua_pushinteger(L, i+1);//index of contour

		Contour** pcnt = (Contour**)lua_newuserdata(L, sizeof(Contour*));
		*pcnt = cnts[i];
		luaL_getmetatable(L, CONTOUR_MT);
		lua_setmetatable(L, -2);


		lua_settable(L, -3);
		
	}

	return 1;
}
//-------------------------------------------------------------------------------


//IMAGE
//-------------------------------------------------------------------------------
static int lua_set_at(lua_State* L) {
	Image** pimg 	= luaL_checkudata(L, 1, IMAGE_MT);
	size_t 	c 		= luaL_checkinteger(L, 2) -1;
	size_t 	x 		= luaL_checkinteger(L, 3) -1;
	size_t 	y 		= luaL_checkinteger(L, 4) -1;
	size_t 	v 		= luaL_checkinteger(L, 5);
	if (v > MAX_VALUE)
		v = MAX_VALUE;
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
			return luaL_error(L, "memory error");

		for (int i = 1; i <= n_lower; i++) {
			//tables are at 2 and 3 pushing index to get value
			get_by_index(L, 2, i);
			lowers[i-1] = lua_tointeger(L, -1);

			get_by_index(L, 3, i);
			uppers[i-1] = lua_tointeger(L, -1);

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

static int lua_image_shape(lua_State* L) {
	Image** pimg = luaL_checkudata(L, 1, IMAGE_MT);
	lua_pushinteger(L, (*pimg)->channels);
	lua_pushinteger(L, (*pimg)->width);
	lua_pushinteger(L, (*pimg)->height);
	
	return 3;
}

static int lua_add_image(lua_State* L) {
	int add(value_t current, float x) {
		return (int)((float)current + x);
	}
	lua_arth_image(L, &add);
	return 1;
}

static int lua_sub_image(lua_State* L) {
	int sub(value_t current, float x) {
		return (int)((float)current - x);
	}
	lua_arth_image(L, &sub);
	return 1;
}

static int lua_mul_image(lua_State* L) {
	int mul(value_t current, float x) {
		return (int)((float)current * x);
	}
	lua_arth_image(L, &mul);
	return 1;
}

static int lua_div_image(lua_State* L) {
	int div(value_t current, float x) {
		return (int)((float)current / x);
	}
	lua_arth_image(L, &div);
	return 1;
}

static int lua_gc_image(lua_State* L) {
	Image** pimg = (Image**)luaL_checkudata(L, 1, IMAGE_MT);
	free_image(*pimg);
	return 0;
}
//-------------------------------------------------------------------------------

//DEVICE
//-------------------------------------------------------------------------------
static int lua_read_frame(lua_State* L) {
	Device** pdev = (Device**)luaL_checkudata(L, 1, DEVICE_MT);
	Image** pimg = (Image**)lua_newuserdata(L, sizeof(Image*));

	*pimg = read_frame(*pdev);

	luaL_getmetatable(L, IMAGE_MT);
	lua_setmetatable(L, -2);

	return 1;
}

static int lua_close_device(lua_State* L) {
	Device** pdev = (Device**)luaL_checkudata(L, 1, DEVICE_MT);
	free_device(*pdev);
	return 0;
}


static int lua_device_resolution(lua_State* L) {
	Device** pdev = (Device**)luaL_checkudata(L, 1, DEVICE_MT);
	lua_pushinteger(L, (*pdev)->fmt->fmt.pix.width);
	lua_pushinteger(L, (*pdev)->fmt->fmt.pix.height);
	return 2;
}
//-------------------------------------------------------------------------------

//CONTOUR
//-------------------------------------------------------------------------------

static int lua_contour_center(lua_State* L) {
	Contour** pcnt = (Contour**)luaL_checkudata(L, 1, CONTOUR_MT);
	float x, y;
	contour_center(*pcnt, &x, &y);

	lua_createtable(L, 2, 0);

	lua_pushinteger(L, 1);//X index
	lua_pushnumber(L, x +0.5); //adds 0.5 because lua counts from 1 --> 1 pixel /2 = 0.5
	lua_settable(L, 2);

	lua_pushinteger(L, 2);//X index
	lua_pushnumber(L, y +0.5);
	lua_settable(L, 2);
	return 1;
}

static int lua_is_inside(lua_State* L) {
	Contour** pcnt = (Contour**)luaL_checkudata(L, 1, CONTOUR_MT);

	luaL_checktype(L, 2, LUA_TTABLE);
	size_t n = luaL_len(L, 2);
	if (n == 2) {
		get_by_index(L, 2, 1);
		float x = lua_tonumber(L, -1) -1;
		if (x < 0) {
			lua_pushboolean(L, 0);
			return 1;
		}
		

		get_by_index(L, 2, 2);
		float y = lua_tonumber(L, -1) -1;
		if (y < 0) {
			lua_pushboolean(L, 0);
			return 1;
		}

		char result = is_inside_contour(*pcnt, x, y);
		lua_pushboolean(L, result);
		return 1;
	}
	else {
		luaL_error(L, "2d point expected");
		return 0;
	}
}

static int lua_contour_to_table(lua_State* L) {
	Contour** pcnt = (Contour**)luaL_checkudata(L, 1, CONTOUR_MT);

	lua_createtable(L, (*pcnt)->index, 1);
	for (int i = 0; i < (*pcnt)->index; i++) {
		lua_pushinteger(L, i+1);//index

		lua_createtable(L, 2, 0);
		lua_pushinteger(L, 1);//X index
		lua_pushinteger(L, (*pcnt)->Xi[i] +1);
		lua_settable(L, -3);

		lua_pushinteger(L, 2);//Y index
		lua_pushinteger(L, (*pcnt)->Yi[i] +1);
		lua_settable(L, -3);
		
		lua_settable(L, -3);
	}
	return 1;
}

static int lua_contour_extreme_points(lua_State* L) {
	Contour** pcnt = (Contour**)luaL_checkudata(L, 1, CONTOUR_MT);

	size_t* x = calloc(4, sizeof(size_t));
	size_t* y = calloc(4, sizeof(size_t));

	get_contour_extreme(*pcnt, x, y);

	lua_createtable(L, 4, 1);
	for (int i = 0; i < 4; i++) {
		lua_pushinteger(L, i+1);//index

		lua_createtable(L, 2, 0);

		lua_pushinteger(L, 1);//X index
		lua_pushinteger(L, x[i] +1);
		lua_settable(L, -3);

		lua_pushinteger(L, 2);//Y index
		lua_pushinteger(L, y[i] +1);
		lua_settable(L, -3);
		
		lua_settable(L, -3);
	}
	return 1;
}

static int lua_contour_area(lua_State* L) {
	Contour** pcnt = (Contour**)luaL_checkudata(L, 1, CONTOUR_MT);
	lua_pushinteger(L, get_contour_area(*pcnt));
	return 1;

}

static int lua_gc_contour(lua_State* L) {
	Contour** pcnt = (Contour**)luaL_checkudata(L, 1, CONTOUR_MT);
	free_contour(*pcnt);
	return 0;
}
//-------------------------------------------------------------------------------

//LUA
//-------------------------------------------------------------------------------
int LUA_API luaopen_kestrel(lua_State* L) {
	const luaL_Reg lib[] = {
		{"newimage",			lua_new_image},
		{"read_rgb_pixelmap",	lua_read_rgb_pixel_map},
		{"rgb_to_hsv", 			lua_rgb_to_hsv},
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
				{"shape", 				lua_image_shape},
				{"__add", 				lua_add_image},
				{"__sub", 				lua_sub_image},
				{"__mul", 				lua_mul_image},
				{"__div", 				lua_div_image},
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
				{"resolution", 	lua_device_resolution},
				{NULL, NULL},
			};
		luaL_setfuncs(L, device_funcs, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}

	lua_pop(L, 1);

	if (luaL_newmetatable(L, CONTOUR_MT)) {
		const luaL_Reg contour_funcs[] = {
				{"center",		lua_contour_center},
				{"is_inside",	lua_is_inside},
				{"totable",		lua_contour_to_table},
				{"extreme",		lua_contour_extreme_points},
				{"area",		lua_contour_area},
				{"__gc",		lua_gc_contour},
				{NULL, NULL},
			};
		luaL_setfuncs(L, contour_funcs, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}

	lua_pop(L, 1); 

	lua_createtable(L, sizeof(lib) / sizeof(lib[0]), 0);
	luaL_setfuncs(L, lib, 0);
	return 1;
}
//-------------------------------------------------------------------------------
