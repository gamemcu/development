
// Module for interfacing with gamemcu libs

#include "module.h"
#include "lauxlib.h"
#include "platform.h"

#include "task/task.h"
#include "gm_input.h"
#include "gm_lcd.h"
#include "gm_mpu.h"
#include "gm_sound.h"
#include "gm_led.h"

#include "gm_label.h"
#include "gm_sprite.h"
#include "gm_titlemap.h"
#include "gm_starsky.h"
#include "gm_layer.h"
#include "gm_scene.h"
#include "gm_listener.h"
#include "gm_director.h"
#include "gm_camera.h"

#define GM_CHECK(a, err_msg) \
  if (!(a)){ \
    luaL_error( L, err_msg ); \
    return 0; \
  }

#define _SET_INIT_FIELD(input, a, b) \
  lua_pushboolean (L, (input) & (1 << (a))); \
  lua_setfield( L, -2, (b) );

#define _INPUT_NEW_TABLE(input) \
  do{ \
    lua_newtable(L);\
    _SET_INIT_FIELD(input, GM_INPUT_UP, "up"); \
    _SET_INIT_FIELD(input, GM_INPUT_DOWN, "down"); \
    _SET_INIT_FIELD(input, GM_INPUT_LEFT, "left"); \
    _SET_INIT_FIELD(input, GM_INPUT_RIGHT, "right"); \
    _SET_INIT_FIELD(input, GM_INPUT_A, "a"); \
    _SET_INIT_FIELD(input, GM_INPUT_B, "b"); \
    _SET_INIT_FIELD(input, GM_INPUT_C, "c"); \
    _SET_INIT_FIELD(input, GM_INPUT_D, "d"); \
  }while(0)

typedef struct
{
  gm_node_t* ptr;
  uint8_t is_added;
} lgm_node_t;

static task_handle_t main_task_handle, isr_task_handle, listener_task_handle, schduler_task_handle;

static int lgm_sound_tone( lua_State *L )
{
  int freq = luaL_checkint( L, 1 );
  int dura = luaL_checkint( L, 2 );
  gm_sound_tone(freq, dura);
  return 0;
}

static int lgm_led( lua_State *L )
{
  int stack = 0;
  gm_led_cfg_t cfg;
  memset(&cfg, 0, sizeof(gm_led_cfg_t));
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "color");
  if (lua_istable(L, -1)) {
    luaL_checkanytable(L, -1);
    lua_pushnil(L);
    while (lua_next(L, 2)) {
      cfg.data[cfg.len++ % LED_DATA_LEN] = luaL_checkint(L, -1);
      lua_pop(L, 1);
    }
  }
  lua_getfield (L, stack, "dura");
  cfg.dura = luaL_optint(L, -1, -1);
  if (cfg.dura && cfg.dura < 10) {
    cfg.dura = 10;
  }
  lua_getfield (L, stack, "mode");
  cfg.mode = luaL_optint(L, -1, GM_LED_NORMAL);
  lua_getfield (L, stack, "freq");
  cfg.freq = luaL_optint(L, -1, 0);
  gm_led_run(&cfg);
  return 0;
}

static int lgm_mpu_setup( lua_State *L )
{
  gm_mpu_cfg_t cfg;
  luaL_checkanytable (L, 1);
  lua_getfield (L, 1, "i2c");
  cfg.i2c = luaL_checkint( L, -1 );
  GM_CHECK((cfg.i2c >= I2C_NUM_0) && (cfg.i2c < I2C_NUM_MAX), "invalid i2c\n");
  lua_getfield (L, 1, "intr");
  cfg.in = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "sclk");
  cfg.sclk = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "sda");
  cfg.sda = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "speed");
  cfg.speed = luaL_checkint( L, -1 );
  GM_CHECK((cfg.speed >= 400000) && (cfg.speed <= 1000000), "invalid speed, 400000 <= speed <= 1000000\n");
  cfg.handle = isr_task_handle;
  lua_getfield (L, 1, "reinit");
  cfg.reinit = luaL_optint( L, -1, 0 );
  gm_mpu_setup(&cfg);
  return 0;
}

static int lgm_mpu_read_accel( lua_State *L )
{
  mpud_result2_t *result = gm_mpu_read();
  if (result) {
    lua_pushnumber(L, result->accel[0]);
    lua_pushnumber(L, result->accel[1]);
    lua_pushnumber(L, result->accel[2]);
    return 3;
  }
  return 0;
}

static int lgm_mpu_read_gyro( lua_State *L )
{
  mpud_result2_t *result = gm_mpu_read();
  if (result) {
    lua_pushnumber(L, result->gyro[0]);
    lua_pushnumber(L, result->gyro[1]);
    lua_pushnumber(L, result->gyro[2]);
    return 3;
  }
  return 0;
}

static int lgm_mpu_read_angle( lua_State *L )
{
  mpud_result2_t *result = gm_mpu_read_angle();
  if (result) {
    lua_pushnumber(L, result->pitch);
    lua_pushnumber(L, result->roll);
    lua_pushnumber(L, result->yaw);
    return 3;
#if 0
    lua_createtable(L, 3, 0);
    if (result->sensors & INV_WXYZ_QUAT ) {
      lua_pushstring(L, "angle");
      lua_createtable(L, 0, 3);
      lua_pushnumber(L, result->pitch);
      lua_setfield(L, -2, "pitch");
      lua_pushnumber(L, result->roll);
      lua_setfield(L, -2, "roll");
      lua_pushnumber(L, result->yaw);
      lua_setfield(L, -2, "yaw");
      lua_settable(L, -3);
    }
    if (result->sensors & INV_XYZ_GYRO) {
      lua_pushstring(L, "gyro");
      lua_createtable(L, 0, 3);
      lua_pushnumber(L, result->gyro[0]);
      lua_setfield(L, -2, "x");
      lua_pushnumber(L, result->gyro[1]);
      lua_setfield(L, -2, "y");
      lua_pushnumber(L, result->gyro[2]);
      lua_setfield(L, -2, "z");
      lua_settable(L, -3);
    }
    if (result->sensors & INV_XYZ_ACCEL) {
      lua_pushstring(L, "accel");
      lua_createtable(L, 0, 3);
      lua_pushnumber(L, result->accel[0]);
      lua_setfield(L, -2, "x");
      lua_pushnumber(L, result->accel[1]);
      lua_setfield(L, -2, "y");
      lua_pushnumber(L, result->accel[2]);
      lua_setfield(L, -2, "z");
      lua_settable(L, -3);
    }
    return 1;
#endif
  }
  return 0;
}

static int lgm_input_setup( lua_State *L )
{
  gm_input_cfg_t cfg;
  luaL_checkanytable (L, 1);
  lua_getfield (L, 1, "x");
  cfg.x = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "y");
  cfg.y = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "a");
  cfg.a = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "b");
  cfg.b = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "c");
  cfg.c = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "d");
  cfg.d = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "reinit");
  cfg.reinit = luaL_optint( L, -1, 0);
  gm_input_setup(&cfg);
  return 0;
}

static int lgm_input_read( lua_State *L )
{
  int16_t input = gm_input_read();
  if (input > -1) {
    _INPUT_NEW_TABLE(input);
    return 1;
  }
  return 0;
}
static int lgm_lcd_setup( lua_State *L )
{
  gm_lcd_cfg_t cfg;
  luaL_checkanytable (L, 1);
  lua_getfield (L, 1, "host");
  cfg.host = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "sclk");
  cfg.sclk = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "mosi");
  cfg.mosi = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "cs");
  cfg.cs = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "dc");
  cfg.dc = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "rst");
  cfg.rst = luaL_checkint( L, -1 );
  lua_getfield (L, 1, "reinit");
  cfg.reinit = luaL_optint( L, -1, 0);
  GM_CHECK(gm_lcd_setup(&cfg) == ESP_OK, "fail to init lcd\n");
  return 0;
}

static int lgm_lcd_clear_buffer( lua_State *L )
{
  gm_lcd_clear_buffer();
  return 0;
}

static int lgm_lcd_send_buffer( lua_State *L )
{
  GM_CHECK(gm_lcd_send_buffer() == ESP_OK, "fail to send buffer\n");
  return 0;
}

static uint8_t _check_bmp( lua_State *L, int stack, bitmap2_t *bmp, int rendermode )
{
  size_t len;
  const char *data = luaL_checklstring(L, stack, &len);
  bmp->data = (uint8_t*)data;
  bmp->len = len;
  bmp->rendermode = rendermode;
  uint8_t width = data[0];
  uint8_t height = data[1];
  uint8_t frames = data[2];
  uint8_t color = data[3];
  len -= 4;
  bmp->framesize = _ASSET_SIZE(width, height);
  uint16_t tmp = bmp->framesize * frames;
  uint16_t size = tmp;
  if ((color & DRAW_GRAY) || (bmp->rendermode & DRAW_GRAY)) {
    bmp->rendermode |= DRAW_GRAY;
    size += tmp;
  }
  if ((color & DRAW_MASK) || (bmp->rendermode & DRAW_MASK)) {
    bmp->rendermode |= DRAW_MASK;
    size += tmp;
  }
  if (len != size) {
    luaL_error(L, "bitmap format err! len:%d,width:%d,height:%d,frames:%d", len, width, height, frames); \
    return 0;
  }
  return 1;
}

#define CHECK_BMP() \
  bitmap2_t bmp; \
  bmp.data = (uint8_t*)data; \
  bmp.len = len; \
  bmp.rendermode = rendermode; \
  uint8_t width = data[0]; \
  uint8_t height = data[1]; \
  uint8_t frames = data[2]; \
  uint8_t color = data[3]; \
  len -= 4; \
  bmp.framesize = _ASSET_SIZE(width, height); \
  uint16_t tmp = bmp.framesize * frames; \
  uint16_t size = tmp; \
  if ((color & DRAW_GRAY) || (bmp.rendermode & DRAW_GRAY)){ \
    bmp.rendermode |= DRAW_GRAY; \
    size += tmp; \
  } \
  if ((color & DRAW_MASK) || (bmp.rendermode & DRAW_MASK)){ \
    bmp.rendermode |= DRAW_MASK; \
    size += tmp; \
  } \
  if (len != size){ \
    luaL_error(L, "bitmap format err! len:%d,width:%d,height:%d,frames:%d", len, width, height, frames); \
    return 0; \
  }

static int lgm_lcd_write_ascii( lua_State *L )
{
  int stack = 0;
  const char *str = luaL_checkstring(L, ++stack);
  int x = luaL_optint(L, ++stack, 0);
  int y = luaL_optint(L, ++stack, 0);
  gm_lcd_write_ascii(x, y, str);
  gm_lcd_send_buffer();
  return 0;
}

static int lgm_lcd_draw_bmp( lua_State *L )
{
  int stack = 0;
  size_t len;
  const char *data = luaL_checklstring(L, ++stack, &len);
  int x = luaL_checkint(L, ++stack);
  int y = luaL_checkint(L, ++stack);
  int i = luaL_optint(L, ++stack, 0);
  int rendermode = luaL_optint(L, ++stack, DRAW_NORMAL);
  CHECK_BMP();
  gm_lcd_draw_bmp2(&bmp, x, y, i);
  gm_lcd_send_buffer();
  return 0;
}

static int lgm_lcd_draw_point( lua_State *L )
{
  int stack = 0;
  int x = luaL_checkint(L, ++stack);
  int y = luaL_checkint(L, ++stack);
  int color = luaL_optint(L, ++stack, 255);
  gm_lcd_draw_point(x, y, color);
  gm_lcd_send_buffer();
  return 0;
}

static int lgm_node_free( lua_State *L )
{
  int stack = 0;
  if (lua_isuserdata(L, ++stack)) {
    lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
    GM_UNREF(lnode->ptr);
  }
  return 0;
}

static uint8_t lgm_node_create( lua_State *L, gm_node_t* node, const char* t)
{
  lgm_node_t* lnode = (lgm_node_t*)lua_newuserdata(L, sizeof(lgm_node_t));
  if (lnode == NULL) {
    return 0;
  }
  lnode->ptr = node;
  lnode->is_added = 0;
  GM_REF(node);
  /* set its metatable */
  luaL_getmetatable(L, t);
  lua_setmetatable(L, -2);
  return 1;
}

static int lgm_label_create( lua_State *L )
{
  int stack = 0;
  const char* str = luaL_checkstring(L, ++stack);
  int x = luaL_optint(L, ++stack, 0);
  int y = luaL_optint(L, ++stack, 0);
  int z = luaL_optint(L, ++stack, 0);
  gm_node_t* node = gm_label_create(x, y, z, str);
  GM_CHECK(node != NULL, "fail to create label\n");
  GM_CHECK(lgm_node_create(L, node, "gm.label"), "fail to crate label node\n");
  return 1;
}

static int lgm_label_id( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.label");
  lua_pushnumber(L, (uint32_t)lnode->ptr->p);
  return 1;
}

static int lgm_label_set_visiable( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.label");
  int v = luaL_checkint(L, ++stack);
  GM_CHECK(gm_label_set_visiable(lnode->ptr->p, v), "fail to set visiable\n");
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_label_get_pos( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.label");
  int32_t x, y;
  if (gm_label_get_pos(lnode->ptr->p, &x, &y)) {
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
  }
  return 0;
}

static int lgm_label_set_str( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.label");
  const char* str = luaL_checkstring(L, ++stack);
  GM_CHECK(gm_label_set_str(lnode->ptr->p, str), "fail to set str\n");
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_label_set_speed( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.label");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int xspeed = luaL_checkint(L, -1);
    gm_label_set_xspeed(lnode->ptr->p, xspeed);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int yspeed = luaL_checkint(L, -1);
    gm_label_set_yspeed(lnode->ptr->p, yspeed);
  }
  lua_pushvalue(L, 1);
  return 1;
}


static int lgm_sprite_create( lua_State *L )
{
  int stack = 0;
  size_t len;
  const char *data = luaL_checklstring(L, ++stack, &len);
  int x = luaL_optint(L, ++stack, 0);
  int y = luaL_optint(L, ++stack, 0);
  int z = luaL_optint(L, ++stack, 0);
  int rendermode = luaL_optint(L, ++stack, DRAW_NORMAL);
  CHECK_BMP();
  gm_node_t* node = gm_sprite_create(x, y, z, &bmp);
  GM_CHECK(node != NULL, "fail to create sprite\n");
  GM_CHECK(lgm_node_create(L, node, "gm.sprite"), "fail to create sprite node\n");
  return 1;
}

static int lgm_sprite_set_visiable( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  int v = luaL_checkint(L, ++stack);
  GM_CHECK(gm_sprite_set_visiable(lnode->ptr->p, v), "fail to set visiable\n");
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_sprite_id( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  lua_pushnumber(L, (uint32_t)lnode->ptr->p);
  return 1;
}

static int lgm_sprite_set_max( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int max = luaL_checkint(L, -1);
    gm_sprite_set_xmax(lnode->ptr->p, max);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int max = luaL_checkint(L, -1);
    gm_sprite_set_ymax(lnode->ptr->p, max);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_sprite_set_min( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int min = luaL_checkint(L, -1);
    gm_sprite_set_xmin(lnode->ptr->p, min);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int min = luaL_checkint(L, -1);
    gm_sprite_set_ymin(lnode->ptr->p, min);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_sprite_set_pos( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int x = luaL_checkint(L, -1);
    gm_sprite_set_xpos(lnode->ptr->p, x);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int y = luaL_checkint(L, -1);
    gm_sprite_set_ypos(lnode->ptr->p, y);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_sprite_get_pos( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  int32_t x, y;
  if (gm_sprite_get_pos(lnode->ptr->p, &x, &y)) {
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
  }
  return 0;
}

static int lgm_sprite_set_frame( lua_State *L )
{
  int stack = 0;
  uint8_t ret = 1;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  int frame = luaL_checkint(L, ++stack);
  if (!gm_sprite_set_frame(lnode->ptr->p, frame)) {
    ret = 0;
  }
  lua_pushvalue(L, 1);
  lua_pushboolean(L, ret);
  return 2;
}

static int lgm_sprite_get_frame( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  uint8_t frame = gm_sprite_get_frame(lnode->ptr->p);
  lua_pushinteger(L, frame);
  return 1;
}

static int lgm_sprite_get_speed( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  int16_t x, y;
  GM_CHECK(gm_sprite_get_speed(lnode->ptr->p, &x, &y), "fail to get speed\n");
  lua_newtable(L);
  lua_pushinteger(L, x);
  lua_setfield(L, -2, "x");
  lua_pushinteger(L, y);
  lua_setfield(L, -2, "y");
  return 1;
}

static int lgm_sprite_set_maxspeed( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int max = luaL_checkint(L, -1);
    gm_sprite_set_max_xspeed(lnode->ptr->p, max);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int max = luaL_checkint(L, -1);
    gm_sprite_set_max_yspeed(lnode->ptr->p, max);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_sprite_set_speed( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int xspeed = luaL_checkint(L, -1);
    gm_sprite_set_xspeed(lnode->ptr->p, xspeed);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int yspeed = luaL_checkint(L, -1);
    gm_sprite_set_yspeed(lnode->ptr->p, yspeed);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_sprite_set_acc( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int xacc = luaL_checkint(L, -1);
    gm_sprite_set_xacc(lnode->ptr->p, xacc);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int yacc = luaL_checkint(L, -1);
    gm_sprite_set_yacc(lnode->ptr->p, yacc);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_sprite_set_friction( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lnode = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.sprite");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int xfrict = luaL_checkint(L, -1);
    gm_sprite_set_xfriction(lnode->ptr->p, xfrict);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int yfrict = luaL_checkint(L, -1);
    gm_sprite_set_yfriction(lnode->ptr->p, yfrict);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_titlemap_create( lua_State *L )
{
  bitmap2_t title, map;
  int z = luaL_optint(L, 3, 0);
  int rendermode = luaL_optint(L, 4, DRAW_NORMAL);
  GM_CHECK(_check_bmp(L, 1, &title, rendermode), "fail to check title\n");
  GM_CHECK((title.data[BITMAP_WIDTH] == title.data[BITMAP_HEIGHT]) && (title.data[BITMAP_WIDTH] % 8 == 0), "title format err\n");
  GM_CHECK(_check_bmp(L, 2, &map, DRAW_NORMAL), "fail to check map\n");
  gm_node_t *node = gm_titlemap_create(&title, &map, z);
  GM_CHECK(node != NULL, "fail to create_titlemap\n");
  GM_CHECK(lgm_node_create(L, node, "gm.titlemap"), "fail to create titlemap node\n");
  return 1;
}

static int lgm_starsky_create( lua_State *L )
{
  int stack = 0;
  int n = luaL_checkint(L, ++stack);
  int z = luaL_optint(L, ++stack, 0);
  int color = luaL_optint(L, ++stack, 255);
  gm_node_t *node = gm_starsky_create(n, z, color);
  GM_CHECK(node != NULL, "fail to create starsky\n");
  GM_CHECK(lgm_node_create(L, node, "gm.starsky"), "fail to create starsky node\n");
  return 1;
}

static int lgm_starsky_set_speed( lua_State *L )
{
  int stack = 0;
  lgm_node_t *lstarsky = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.starsky");
  luaL_checkanytable (L, ++stack);
  lua_getfield (L, stack, "x");
  if (!lua_isnil(L, -1)) {
    int xspeed = luaL_checkint(L, -1);
    gm_starsky_set_xspeed(lstarsky->ptr->p, xspeed);
  }
  lua_getfield (L, stack, "y");
  if (!lua_isnil(L, -1)) {
    int yspeed = luaL_checkint(L, -1);
    gm_starsky_set_yspeed(lstarsky->ptr->p, yspeed);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_layer_create( lua_State * L )
{
  int stack = 0;
  int x = luaL_optint(L, ++stack, 0);
  int y = luaL_optint(L, ++stack, 0);
  int z = luaL_optint(L, ++stack, 0);
  gm_node_t* node = gm_layer_create(x, y, z);
  GM_CHECK(node != NULL, "fail to create layer\n");
  GM_CHECK(lgm_node_create(L, node, "gm.layer"), "fail to create layer node\n");
  return 1;
}

static int lgm_layer_add( lua_State * L )
{
  int stack = 0;
  lgm_node_t *llayer = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.layer");
  int n = lua_gettop(L) - 1;
  while (n-- > 0) {
    if (lua_isuserdata(L, ++stack)) {
      lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
      if (!lnode->is_added) {
        GM_CHECK(gm_layer_add(llayer->ptr->p, lnode->ptr), "fail to add\n");
        lnode->is_added = 1;
      }
    }
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_layer_remove( lua_State * L )
{
  int stack = 0;
  lgm_node_t *llayer = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.layer");
  int n = lua_gettop(L) - 1;
  while (n-- > 0) {
    if (lua_isuserdata(L, ++stack)) {
      lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
      if (lnode->is_added) {
        GM_CHECK(gm_layer_remove(llayer->ptr->p, lnode->ptr), "fail to remove\n");
        lnode->is_added = 0;
      }
    }
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_layer_set_z( lua_State * L )
{
  int stack = 0;
  lgm_node_t *llayer = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.layer");
  if (lua_isuserdata(L, ++stack)) {
    lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
    int z = luaL_optint(L , ++stack, 0);
    gm_layer_set_z(llayer->ptr->p, lnode->ptr, z);
  }
  lua_pushvalue(L, 1);
  return 1;
}


static void _unref( int ref )
{
  lua_State *L = lua_getstate ();
  luaL_unref (L, LUA_REGISTRYINDEX, ref);
}

static int lgm_listener_create( lua_State * L )
{
  int stack = 0;
  int type = luaL_checkint(L , ++stack);
  task_handle_t handle;
  if (GM_EVENT_INPUT == type) {
    handle = listener_task_handle;
  }
  else {
    luaL_error(L, "event not exist\n");
    return 0;
  }
  luaL_checkanyfunction (L, ++stack);
  int cb_ref = luaL_ref (L, LUA_REGISTRYINDEX);
  int prio = luaL_optint(L , ++stack, 0);
  gm_task_t task = {
    .type = type,
    .cb_ref = cb_ref,
    .unref = _unref,
    .handle = handle,
  };
  gm_node_t *node = gm_listener_create(prio, &task);
  if ((node == NULL) || (!lgm_node_create(L, node, "gm.node"))) {
    luaL_unref (L, LUA_REGISTRYINDEX, cb_ref);
    if (node == NULL) {
      luaL_error(L, "fail to create listener\n");
    }
    else {
      luaL_error(L, "fail to create listener node\n");
    }
    return 0;
  }
  return 1;
}

static int lgm_schduler_create( lua_State * L )
{
  int stack = 0;
  int type = luaL_checkint(L , ++stack);
  GM_CHECK(type < GM_SCH_COUNT, "ERR! invalid type \n");
  int interval = luaL_checkint(L , ++stack);
  GM_CHECK((interval >= 10) && (interval < GM_SCH_MAX_INTERVAL), "ERR! 10 <= interval <= 6870947\n");
  luaL_checkanyfunction (L, ++stack);
  int cb_ref = luaL_ref (L, LUA_REGISTRYINDEX);
  int prio = luaL_optint(L , ++stack, 0);
  gm_task_t task = {
    .type = type,
    .cb_ref = cb_ref,
    .unref = _unref,
    .handle = schduler_task_handle
  };
  gm_node_t* node = gm_schduler_create(prio, interval, &task);
  if ((node == NULL) || (!lgm_node_create(L, node, "gm.node"))) {
    luaL_unref (L, LUA_REGISTRYINDEX, cb_ref);
    if (node == NULL) {
      luaL_error(L, "fail to create schduler\n");
    }
    else {
      luaL_error(L, "fail to create schduler node\n");
    }
    return 0;
  }
  return 1;
}

static int lgm_scene_create( lua_State * L )
{
  gm_node_t* node = gm_scene_create();
  GM_CHECK(node != NULL, "fail to create scene\n");
  GM_CHECK(lgm_node_create(L, node, "gm.scene"), "fail to create scene node\n");
  return 1;
}

static int lgm_scene_add( lua_State * L )
{
  int stack = 0;
  lgm_node_t *lscene = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.scene");
  int n = lua_gettop(L) - 1;
  while (n-- > 0) {
    if (lua_isuserdata(L, ++stack)) {
      lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
      if (!lnode->is_added) {
        GM_CHECK(gm_scene_add(lscene->ptr->p, lnode->ptr), "fail to add\n");
        lnode->is_added = 1;
      }
    }
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_scene_remove( lua_State * L )
{
  int stack = 0;
  lgm_node_t *lscene = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.scene");
  int n = lua_gettop(L) - 1;
  while (n-- > 0) {
    if (lua_isuserdata(L, ++stack)) {
      lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
      if (lnode->is_added) {
        GM_CHECK(gm_scene_remove(lscene->ptr->p, lnode->ptr), "fail to remove");
        lnode->is_added = 0;
      }
    }
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_scene_set_z( lua_State * L )
{
  int stack = 0;
  lgm_node_t *lscene = (lgm_node_t *)luaL_checkudata(L, ++stack, "gm.scene");
  if (lua_isuserdata(L, ++stack)) {
    lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
    int z = luaL_optint(L , ++stack, 0);
    gm_scene_set_z(lscene->ptr->p, lnode->ptr, z);
  }
  lua_pushvalue(L, 1);
  return 1;
}

static int lgm_director_set_fps( lua_State * L )
{
  int stack = 0;
  int fps = luaL_checkint(L , ++stack);
  GM_CHECK(fps && (fps <= GM_MAX_FPS), "invalid fps\n");
  gm_director_set_fps(fps);
  return 0;
}

static int lgm_director_get_fps( lua_State * L )
{
  uint16_t fps = gm_director_get_fps();
  lua_pushinteger(L, fps);
  return 1;
}

static int lgm_director_show_fps( lua_State * L )
{
  int stack = 0;
  int en = luaL_checkint(L , ++stack);
  gm_director_show_fps(en);
  return 0;
}

static int lgm_director_replace_scene( lua_State * L )
{
  int stack = 0;
  if (lua_isuserdata(L, ++stack)) {
    lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
    GM_CHECK(gm_director_replace_scene(lnode->ptr), "fail to replace_scene\n");
    task_post_low (main_task_handle, 0);
  }
  return 0;
}

static int lgm_camera_focus( lua_State * L )
{
  int stack = 0;
  if (lua_isuserdata(L, ++stack)) {
    lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
    int xoffset = luaL_checkint(L , ++stack);
    int yoffset = luaL_checkint(L , ++stack);
    GM_CHECK(gm_camera_focus(lnode->ptr, xoffset, yoffset), "fail to focus node\n");
  }
  return 0;
}

static int lgm_camera_unfocus( lua_State * L )
{
  int stack = 0;
  if (lua_isuserdata(L, ++stack)) {
    lgm_node_t *lnode = (lgm_node_t*)lua_touserdata(L, stack);
    GM_CHECK(gm_camera_unfocus(lnode->ptr), "fail to unfocus node\n");
  }
  return 0;
}

static int lgm_camera_get_pos( lua_State * L )
{
  lua_pushinteger(L, gm_camera()->x);
  lua_pushinteger(L, gm_camera()->y);
  return 2;
}

typedef struct
{
  int32_t x, y;
  uint16_t width, height;
} collide_element_t;

uint8_t _is_collide(collide_element_t *a, collide_element_t *b)
{
  return !( a->x > b->x + b->width ||
            a->x + a->width < b->x ||
            a->y > b->y + b->height ||
            a->y + a->height < b->y  );
}


static int lgm_collide( lua_State * L )
{
  uint8_t is_collide = 0;
  lgm_node_t *lnode = NULL, *lnode2 = NULL;
  int stack = 0;
  if (lua_isuserdata(L, ++stack)) {
    lnode = (lgm_node_t*)lua_touserdata(L, stack);
  }
  if (lua_isuserdata(L, ++stack)) {
    lnode2 = (lgm_node_t*)lua_touserdata(L, stack);
  }
  if (lnode && lnode2 && (GM_NODE_SPRITE == lnode->ptr->type) && (GM_NODE_SPRITE == lnode2->ptr->type)) {
    gm_sprite_t *sprite = (gm_sprite_t *)lnode->ptr->p;
    collide_element_t e1 = {
      .x = sprite->x.pos >> 7,
      .y = sprite->y.pos >> 7,
      .width = sprite->width,
      .height = sprite->height
    };
    sprite = (gm_sprite_t *)lnode2->ptr->p;
    collide_element_t e2 = {
      .x = sprite->x.pos >> 7,
      .y = sprite->y.pos >> 7,
      .width = sprite->width,
      .height = sprite->height
    };
    if (_is_collide(&e1, &e2)) {
      is_collide = 1;
    }
  }
  lua_pushboolean(L, is_collide);
  return 1;
}

static int lgm_version( lua_State * L )
{
  lua_pushstring(L, GM_VERSION);
  return 1;
}

static const LUA_REG_TYPE lgm_node_map[] = {
  { LSTRKEY( "__gc" ),        LFUNCVAL( lgm_node_free ) },
  { LSTRKEY( "__index" ),     LROVAL( lgm_node_map ) },
  {LNILKEY, LNILVAL}
};

static const LUA_REG_TYPE lgm_label_map[] = {
  { LSTRKEY( "id" ),          LFUNCVAL( lgm_label_id ) },
  { LSTRKEY( "visiable" ),    LFUNCVAL( lgm_label_set_visiable ) },
  { LSTRKEY( "get_pos" ),     LFUNCVAL( lgm_label_get_pos ) },
  { LSTRKEY( "set_speed" ),   LFUNCVAL( lgm_label_set_speed ) },
  { LSTRKEY( "set_str" ),     LFUNCVAL( lgm_label_set_str ) },
  { LSTRKEY( "__gc" ),        LFUNCVAL( lgm_node_free ) },
  { LSTRKEY( "__index" ),     LROVAL( lgm_label_map ) },
  {LNILKEY, LNILVAL}
};

static const LUA_REG_TYPE lgm_sprite_map[] = {
  { LSTRKEY( "visiable" ),    LFUNCVAL( lgm_sprite_set_visiable ) },
  { LSTRKEY( "id" ),          LFUNCVAL( lgm_sprite_id ) },
  { LSTRKEY( "set_min" ),     LFUNCVAL( lgm_sprite_set_min ) },
  { LSTRKEY( "set_max" ),     LFUNCVAL( lgm_sprite_set_max ) },
  { LSTRKEY( "get_pos" ),     LFUNCVAL( lgm_sprite_get_pos ) },
  { LSTRKEY( "set_pos" ),     LFUNCVAL( lgm_sprite_set_pos ) },
  { LSTRKEY( "set_frame" ),   LFUNCVAL( lgm_sprite_set_frame ) },
  { LSTRKEY( "get_frame" ),   LFUNCVAL( lgm_sprite_get_frame ) },
  { LSTRKEY( "get_speed" ),   LFUNCVAL( lgm_sprite_get_speed ) },
  { LSTRKEY( "set_maxspeed" ), LFUNCVAL( lgm_sprite_set_maxspeed ) },
  { LSTRKEY( "set_speed" ),   LFUNCVAL( lgm_sprite_set_speed ) },
  { LSTRKEY( "set_acc" ),     LFUNCVAL( lgm_sprite_set_acc ) },
  { LSTRKEY( "set_friction" ), LFUNCVAL( lgm_sprite_set_friction ) },
  { LSTRKEY( "__gc" ),        LFUNCVAL( lgm_node_free ) },
  { LSTRKEY( "__index" ),     LROVAL( lgm_sprite_map ) },
  {LNILKEY, LNILVAL}
};

static const LUA_REG_TYPE lgm_starsky_map[] = {
  { LSTRKEY( "set_speed" ),   LFUNCVAL( lgm_starsky_set_speed ) },
  { LSTRKEY( "__gc" ),        LFUNCVAL( lgm_node_free ) },
  { LSTRKEY( "__index" ),     LROVAL( lgm_starsky_map ) },
  {LNILKEY, LNILVAL}
};

static const LUA_REG_TYPE lgm_titlemap_map[] = {
  { LSTRKEY( "__gc" ),        LFUNCVAL( lgm_node_free ) },
  { LSTRKEY( "__index" ),     LROVAL( lgm_titlemap_map ) },
  {LNILKEY, LNILVAL}
};

static const LUA_REG_TYPE lgm_layer_map[] = {
  { LSTRKEY( "add" ),           LFUNCVAL( lgm_layer_add ) },
  { LSTRKEY( "remove" ),        LFUNCVAL( lgm_layer_remove ) },
  { LSTRKEY( "set_z" ),         LFUNCVAL( lgm_layer_set_z ) },
  { LSTRKEY( "__gc" ),          LFUNCVAL( lgm_node_free ) },
  { LSTRKEY( "__index" ),       LROVAL( lgm_layer_map ) },
  {LNILKEY, LNILVAL}
};

static const LUA_REG_TYPE lgm_scene_map[] = {
  { LSTRKEY( "add" ),         LFUNCVAL( lgm_scene_add ) },
  { LSTRKEY( "remove" ),      LFUNCVAL( lgm_scene_remove ) },
  { LSTRKEY( "set_z" ),       LFUNCVAL( lgm_scene_set_z ) },
  { LSTRKEY( "__gc" ),        LFUNCVAL( lgm_node_free ) },
  { LSTRKEY( "__index" ),     LROVAL( lgm_scene_map ) },
  {LNILKEY, LNILVAL}
};


static const LUA_REG_TYPE lgm_map[] = {
  { LSTRKEY( "version" ),         LFUNCVAL( lgm_version ) },
  { LSTRKEY( "led" ),             LFUNCVAL( lgm_led ) },
  { LSTRKEY( "tone" ),            LFUNCVAL( lgm_sound_tone ) },
  { LSTRKEY( "setup_mpu" ),       LFUNCVAL( lgm_mpu_setup ) },
  { LSTRKEY( "read_accel" ),      LFUNCVAL( lgm_mpu_read_accel ) },
  { LSTRKEY( "read_angle" ),      LFUNCVAL( lgm_mpu_read_angle ) },
  { LSTRKEY( "read_gyro" ),       LFUNCVAL( lgm_mpu_read_gyro ) },
  { LSTRKEY( "setup_input" ),     LFUNCVAL( lgm_input_setup ) },
  { LSTRKEY( "read_input" ),      LFUNCVAL( lgm_input_read ) },
  { LSTRKEY( "setup_lcd" ),       LFUNCVAL( lgm_lcd_setup ) },
  { LSTRKEY( "write_ascii" ),     LFUNCVAL( lgm_lcd_write_ascii ) },
  { LSTRKEY( "draw_bmp" ),        LFUNCVAL( lgm_lcd_draw_bmp ) },
  { LSTRKEY( "draw_point" ),      LFUNCVAL( lgm_lcd_draw_point ) },
  { LSTRKEY( "clear_buffer" ),    LFUNCVAL( lgm_lcd_clear_buffer ) },
  { LSTRKEY( "send_buffer" ),     LFUNCVAL( lgm_lcd_send_buffer ) },
  { LSTRKEY( "collide" ),         LFUNCVAL( lgm_collide ) },
  { LSTRKEY( "create_listener" ), LFUNCVAL( lgm_listener_create ) },
  { LSTRKEY( "create_schduler" ), LFUNCVAL( lgm_schduler_create ) },
  { LSTRKEY( "create_label" ),    LFUNCVAL( lgm_label_create ) },
  { LSTRKEY( "create_sprite" ),   LFUNCVAL( lgm_sprite_create ) },
  { LSTRKEY( "create_titlemap" ), LFUNCVAL( lgm_titlemap_create ) },
  { LSTRKEY( "create_starsky" ),  LFUNCVAL( lgm_starsky_create ) },
  { LSTRKEY( "create_layer" ),    LFUNCVAL( lgm_layer_create ) },
  { LSTRKEY( "create_scene" ),    LFUNCVAL( lgm_scene_create ) },
  { LSTRKEY( "show_fps" ),        LFUNCVAL( lgm_director_show_fps ) },
  { LSTRKEY( "set_fps" ),         LFUNCVAL( lgm_director_set_fps ) },
  { LSTRKEY( "get_fps" ),         LFUNCVAL( lgm_director_get_fps ) },
  { LSTRKEY( "replace_scene" ),   LFUNCVAL( lgm_director_replace_scene ) },
  { LSTRKEY( "focus" ),           LFUNCVAL( lgm_camera_focus ) },
  { LSTRKEY( "focus" ),           LFUNCVAL( lgm_camera_focus ) },
  { LSTRKEY( "unfocus" ),         LFUNCVAL( lgm_camera_unfocus ) },
  { LSTRKEY( "get_camera_pos" ),  LFUNCVAL( lgm_camera_get_pos ) },
  { LSTRKEY( "INPUT" ),           LNUMVAL( GM_EVENT_INPUT ) },
  { LSTRKEY( "SCH_CONTINUE" ),    LNUMVAL( GM_SCH_CONTINUE ) },
  { LSTRKEY( "SCH_ONCE" ),        LNUMVAL( GM_SCH_ONCE ) },
  { LSTRKEY( "DRAW_NORMAL" ),     LNUMVAL( DRAW_NORMAL ) },
  { LSTRKEY( "DRAW_REVERSE" ),    LNUMVAL( DRAW_REVERSE ) },
  { LSTRKEY( "DRAW_TRANSPARENCY" ), LNUMVAL( DRAW_TRANSPARENCY ) },
  { LSTRKEY( "LED_NORMAL" ),      LNUMVAL( GM_LED_NORMAL ) },
  { LSTRKEY( "LED_BLINK" ),       LNUMVAL( GM_LED_BLINK ) },
  {LNILKEY, LNILVAL}
};

static void gm_main_task_cb(task_param_t param, task_prio_t prio)
{
  (void)param; (void)prio;
  gm_director_main_loop();
  if (!task_post_low (main_task_handle, 0)) {
    // GM_DBG("fail to do main loop\n");
  }
}

static void gm_listener_task_cb(task_param_t param, task_prio_t prio)
{
  (void)prio;
  gm_task_t* task = (gm_task_t*)param;
  if (task->cb_ref == LUA_NOREF) {
    return;
  }
  lua_State *L = lua_getstate ();
  if (GM_EVENT_INPUT == task->type) {
    lua_rawgeti (L, LUA_REGISTRYINDEX, task->cb_ref);
    INPUT_NEW_TABLE(((gm_event_t*)task->info)->input);
    lua_call (L, 1, 0);
  }
}

static void gm_schduler_task_cb(task_param_t param, task_prio_t prio)
{
  (void)prio;
  gm_task_t* task = (gm_task_t*)param;
  if (task->cb_ref == LUA_NOREF) {
    return;
  }
  lua_State *L = lua_getstate ();
  lua_rawgeti (L, LUA_REGISTRYINDEX, task->cb_ref);
  lua_call (L, 0, 0);
}

static void gm_isr_task_cb(task_param_t param, task_prio_t prio)
{
  (void)prio;
  mpud_result2_t* result = (mpud_result2_t*)param;
  if (mpud_read(result) == MPUD_ERR_OK) {
    result->is_valid = 1;
  }
  else {
    result->is_valid = 0;
  }
}

static void gm_board_init_cb(task_param_t param, task_prio_t prio)
{
  (void)param; (void)(prio);
  gm_mpu_cfg_t mpu_cfg = {
    .i2c = I2C_NUM_0,
    .in = 21,
    .sclk = 16,
    .sda = 17,
    .speed = 400000,
    .handle = isr_task_handle
  };
  gm_input_cfg_t input_cfg = {
    .x = 6,
    .y = 7,
    .a = 32,
    .b = 33,
    .c = 26,
    .d = 27
  };
  gm_lcd_cfg_t lcd_cfg = {
    .host = VSPI_HOST,
    .sclk = 18,
    .mosi = 23,
    .cs = 5,
    .dc = 12,
    .rst = 14
  };
  gm_led_setup(4);
  gm_sound_setup(25);
  gm_mpu_setup(&mpu_cfg);
  gm_input_setup(&input_cfg);
  gm_lcd_setup(&lcd_cfg);
  gm_director_show_fps(1);
}

int luaopen_gm( lua_State * L )
{
  main_task_handle = task_get_id (gm_main_task_cb);
  isr_task_handle = task_get_id (gm_isr_task_cb);
  listener_task_handle = task_get_id (gm_listener_task_cb);
  schduler_task_handle = task_get_id (gm_schduler_task_cb);
  task_post_low(task_get_id(gm_board_init_cb), 0);
  task_post_low (main_task_handle, 0);
  luaL_rometatable(L, "gm.node",     (void *)lgm_node_map);
  luaL_rometatable(L, "gm.label",    (void *)lgm_label_map);
  luaL_rometatable(L, "gm.sprite",   (void *)lgm_sprite_map);
  luaL_rometatable(L, "gm.titlemap", (void *)lgm_titlemap_map);
  luaL_rometatable(L, "gm.starsky",  (void *)lgm_starsky_map);
  luaL_rometatable(L, "gm.layer",    (void *)lgm_layer_map);
  luaL_rometatable(L, "gm.scene",    (void *)lgm_scene_map);
  return 0;
}

NODEMCU_MODULE(GM, "gm", lgm_map, luaopen_gm);
