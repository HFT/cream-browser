#include "local.h"

/*!
 * \addtogroup theme
 * @{
 */

static struct theme_t *lua_pushtheme (lua_State *L)
{
     struct theme_t *ret = (struct theme_t *) lua_newuserdata (L, sizeof (struct theme_t));
     luaL_getmetatable (L, LUA_TTHEME);
     lua_setmetatable (L, -2);
     return ret;
}

/* methods */

/*!
 * \fn static int luaL_theme_new (lua_State *L)
 * @param L The lua VM state.
 * @return Number of return value in lua.
 *
 * Create a new lua #theme_t table.
 * \code function theme.new () \endcode
 */
static int luaL_theme_new (lua_State *L)
{
     struct theme_t *t = lua_pushtheme (L);

     t->global.font = g_strdup ("sans normal 8");
     t->global.box  = (struct theme_global_box_t) { FALSE, FALSE, 5 };

     t->statusbar.font      = g_strdup ("sans normal 8");
     t->statusbar.bg.normal = g_strdup ("#000000");
     t->statusbar.bg.secure = g_strdup ("#B0FF00");
     t->statusbar.fg.normal = g_strdup ("#FFFFFF");
     t->statusbar.fg.secure = g_strdup ("#000000");
     t->statusbar.box = (struct theme_box_t) { FALSE, FALSE, FALSE, 0 };

     t->tab.font      = g_strdup ("sans normal 8");
     t->tab.bg.border = g_strdup ("1px solid black");
     t->tab.bg.normal = g_strdup ("#505050");
     t->tab.bg.focus  = g_strdup ("#000000");
     t->tab.fg.border = g_strdup ("1px solid black");
     t->tab.fg.normal = g_strdup ("#CCCCCC");
     t->tab.fg.focus  = g_strdup ("#FFFFFF");
     t->tab.box = (struct theme_box_t) { TRUE, FALSE, FALSE, 0 };

     t->promptbox.font      = g_strdup ("sans normal 8");
     t->promptbox.bg.normal = g_strdup ("#151515");
     t->promptbox.bg.error  = g_strdup ("#FF0000");
     t->promptbox.fg.normal = g_strdup ("#CCCCCC");
     t->promptbox.fg.error  = g_strdup ("#000000");
     t->promptbox.box = (struct theme_box_t) { FALSE, FALSE, FALSE, 0 };

     t->webview.box = (struct theme_box_t) { TRUE, TRUE, TRUE, 0 };

     global.theme = t;
     return 1;
}

static const luaL_reg cream_theme_methods[] =
{
     { "new", luaL_theme_new },
     { NULL, NULL }
};

/* metatable */

/*!
 * \fn static int luaL_theme_gc (lua_State *L)
 * @param L The lua VM state.
 * @return Number of return value in lua.
 *
 * Lua metatable: <code>__gc</code>.
 */
static int luaL_theme_gc (lua_State *L)
{
     struct theme_t *t = (struct theme_t *) lua_touserdata (L, 1);

     g_free (t->global.font);

     g_free (t->statusbar.font);
     g_free (t->statusbar.bg.normal);
     g_free (t->statusbar.bg.secure);
     g_free (t->statusbar.fg.normal);
     g_free (t->statusbar.fg.secure);

     g_free (t->tab.font);
     g_free (t->tab.bg.border);
     g_free (t->tab.bg.normal);
     g_free (t->tab.bg.focus);
     g_free (t->tab.fg.border);
     g_free (t->tab.fg.normal);
     g_free (t->tab.fg.focus);

     g_free (t->promptbox.font);
     g_free (t->promptbox.bg.normal);
     g_free (t->promptbox.bg.error);
     g_free (t->promptbox.fg.normal);
     g_free (t->promptbox.fg.error);

     return 0;
}

static const luaL_reg cream_theme_meta[] =
{
     { "__gc", luaL_theme_gc },
     { NULL, NULL }
};

static const luaI_reg_pre cream_theme_setters[] =
{
     /* section global */
     { "global.font",            luaI_setstring, offsetof (struct theme_t, global.font) },
     { "global.box.padding",     luaI_setint,    offsetof (struct theme_t, global.box.padding) },
     { "global.box.horizontal",  luaI_setbool,   offsetof (struct theme_t, global.box.horizontal) },
     { "global.box.homogeneous", luaI_setbool,   offsetof (struct theme_t, global.box.homogeneous) },

     /* statusbar */
     { "statusbar.font",           luaI_setstring, offsetof (struct theme_t, statusbar.font) },
     { "statusbar.bg.normal",      luaI_setstring, offsetof (struct theme_t, statusbar.bg.normal) },
     { "statusbar.bg.secure",      luaI_setstring, offsetof (struct theme_t, statusbar.bg.secure) },
     { "statusbar.fg.normal",      luaI_setstring, offsetof (struct theme_t, statusbar.fg.normal) },
     { "statusbar.fg.secure",      luaI_setstring, offsetof (struct theme_t, statusbar.fg.secure) },
     { "statusbar.box.pack_start", luaI_setbool,   offsetof (struct theme_t, statusbar.box.pack_start) },
     { "statusbar.box.expand",     luaI_setbool,   offsetof (struct theme_t, statusbar.box.expand) },
     { "statusbar.box.fill",       luaI_setbool,   offsetof (struct theme_t, statusbar.box.fill) },
     { "statusbar.box.padding",    luaI_setint,    offsetof (struct theme_t, statusbar.box.padding) },

     /* tab */
     { "tab.font",           luaI_setstring, offsetof (struct theme_t, tab.font) },
     { "tab.bg.border",      luaI_setstring, offsetof (struct theme_t, tab.bg.border) },
     { "tab.bg.normal",      luaI_setstring, offsetof (struct theme_t, tab.bg.normal) },
     { "tab.bg.focus",       luaI_setstring, offsetof (struct theme_t, tab.bg.focus) },
     { "tab.fg.border",      luaI_setstring, offsetof (struct theme_t, tab.fg.border) },
     { "tab.fg.normal",      luaI_setstring, offsetof (struct theme_t, tab.fg.normal) },
     { "tab.fg.focus",       luaI_setstring, offsetof (struct theme_t, tab.fg.focus) },
     { "tab.box.pack_start", luaI_setbool,   offsetof (struct theme_t, tab.box.pack_start) },
     { "tab.box.expand",     luaI_setbool,   offsetof (struct theme_t, tab.box.expand) },
     { "tab.box.fill",       luaI_setbool,   offsetof (struct theme_t, tab.box.fill) },
     { "tab.box.padding",    luaI_setint,    offsetof (struct theme_t, tab.box.padding) },

     /* promptbox */
     { "promptbox.font",           luaI_setstring, offsetof (struct theme_t, promptbox.font) },
     { "promptbox.bg.normal",      luaI_setstring, offsetof (struct theme_t, promptbox.bg.normal) },
     { "promptbox.bg.error",       luaI_setstring, offsetof (struct theme_t, promptbox.bg.error) },
     { "promptbox.fg.normal",      luaI_setstring, offsetof (struct theme_t, promptbox.fg.normal) },
     { "promptbox.fg.error",       luaI_setstring, offsetof (struct theme_t, promptbox.fg.error) },
     { "promptbox.box.pack_start", luaI_setbool,   offsetof (struct theme_t, promptbox.box.pack_start) },
     { "promptbox.box.expand",     luaI_setbool,   offsetof (struct theme_t, promptbox.box.expand) },
     { "promptbox.box.fill",       luaI_setbool,   offsetof (struct theme_t, promptbox.box.fill) },
     { "promptbox.box.padding",    luaI_setint,    offsetof (struct theme_t, promptbox.box.padding) },

     /* webview */
     { "webview.box.pack_start", luaI_setbool,   offsetof (struct theme_t, webview.box.pack_start) },
     { "webview.box.expand",     luaI_setbool,   offsetof (struct theme_t, webview.box.expand) },
     { "webview.box.fill",       luaI_setbool,   offsetof (struct theme_t, webview.box.fill) },
     { "webview.box.padding",    luaI_setint,    offsetof (struct theme_t, webview.box.padding) },

     { NULL, NULL, 0 }
};

static const luaI_reg_pre cream_theme_getters[] =
{
     /* section global */
     { "global.font",            luaI_getstring, offsetof (struct theme_t, global.font) },
     { "global.box.padding",     luaI_getint,    offsetof (struct theme_t, global.box.padding) },
     { "global.box.horizontal",  luaI_getbool,   offsetof (struct theme_t, global.box.horizontal) },
     { "global.box.homogeneous", luaI_getbool,   offsetof (struct theme_t, global.box.homogeneous) },

     /* statusbar */
     { "statusbar.font",           luaI_getstring, offsetof (struct theme_t, statusbar.font) },
     { "statusbar.bg.normal",      luaI_getstring, offsetof (struct theme_t, statusbar.bg.normal) },
     { "statusbar.bg.secure",      luaI_getstring, offsetof (struct theme_t, statusbar.bg.secure) },
     { "statusbar.fg.normal",      luaI_getstring, offsetof (struct theme_t, statusbar.fg.normal) },
     { "statusbar.fg.secure",      luaI_getstring, offsetof (struct theme_t, statusbar.fg.secure) },
     { "statusbar.box.pack_start", luaI_getbool,   offsetof (struct theme_t, statusbar.box.pack_start) },
     { "statusbar.box.expand",     luaI_getbool,   offsetof (struct theme_t, statusbar.box.expand) },
     { "statusbar.box.fill",       luaI_getbool,   offsetof (struct theme_t, statusbar.box.fill) },
     { "statusbar.box.padding",    luaI_getint,    offsetof (struct theme_t, statusbar.box.padding) },

     /* tab */
     { "tab.font",           luaI_getstring, offsetof (struct theme_t, tab.font) },
     { "tab.bg.border",      luaI_getstring, offsetof (struct theme_t, tab.bg.border) },
     { "tab.bg.normal",      luaI_getstring, offsetof (struct theme_t, tab.bg.normal) },
     { "tab.bg.focus",       luaI_getstring, offsetof (struct theme_t, tab.bg.focus) },
     { "tab.fg.border",      luaI_getstring, offsetof (struct theme_t, tab.fg.border) },
     { "tab.fg.normal",      luaI_getstring, offsetof (struct theme_t, tab.fg.normal) },
     { "tab.fg.focus",       luaI_getstring, offsetof (struct theme_t, tab.fg.focus) },
     { "tab.box.pack_start", luaI_getbool,   offsetof (struct theme_t, tab.box.pack_start) },
     { "tab.box.expand",     luaI_getbool,   offsetof (struct theme_t, tab.box.expand) },
     { "tab.box.fill",       luaI_getbool,   offsetof (struct theme_t, tab.box.fill) },
     { "tab.box.padding",    luaI_getint,    offsetof (struct theme_t, tab.box.padding) },

     /* promptbox */
     { "promptbox.font",           luaI_getstring, offsetof (struct theme_t, promptbox.font) },
     { "promptbox.bg.normal",      luaI_getstring, offsetof (struct theme_t, promptbox.bg.normal) },
     { "promptbox.bg.error",       luaI_getstring, offsetof (struct theme_t, promptbox.bg.error) },
     { "promptbox.fg.normal",      luaI_getstring, offsetof (struct theme_t, promptbox.fg.normal) },
     { "promptbox.fg.error",       luaI_getstring, offsetof (struct theme_t, promptbox.fg.error) },
     { "promptbox.box.pack_start", luaI_getbool,   offsetof (struct theme_t, promptbox.box.pack_start) },
     { "promptbox.box.expand",     luaI_getbool,   offsetof (struct theme_t, promptbox.box.expand) },
     { "promptbox.box.fill",       luaI_getbool,   offsetof (struct theme_t, promptbox.box.fill) },
     { "promptbox.box.padding",    luaI_getint,    offsetof (struct theme_t, promptbox.box.padding) },

     /* webview */
     { "webview.box.pack_start", luaI_getbool,   offsetof (struct theme_t, webview.box.pack_start) },
     { "webview.box.expand",     luaI_getbool,   offsetof (struct theme_t, webview.box.expand) },
     { "webview.box.fill",       luaI_getbool,   offsetof (struct theme_t, webview.box.fill) },
     { "webview.box.padding",    luaI_getint,    offsetof (struct theme_t, webview.box.padding) },

     { NULL, NULL, 0 }
};


/*!
 * \fn int luaL_theme_register (lua_State *L)
 * @param L The lua VM state.
 * @return Number of return value in lua.
 *
 * Register package in the lua VM state.
 */
int luaL_theme_register (lua_State *L)
{
     int metatable, methods;

     luaL_openlib (L, LUA_TTHEME, cream_theme_methods, 0);
     methods = lua_gettop (L);

     luaL_newmetatable (L, LUA_TTHEME);
     luaL_openlib (L, 0, cream_theme_meta, 0);
     metatable = lua_gettop (L);

     lua_pushliteral (L, "__metatable");
     lua_pushvalue (L, methods);
     lua_rawset (L, metatable);

     lua_pushliteral (L, "__index");
     lua_pushvalue (L, metatable);
     luaI_add (L, cream_theme_getters);
     lua_pushvalue (L, methods);
     lua_pushcclosure (L, luaI_index, 2);
     lua_rawset (L, metatable);

     lua_pushliteral (L, "__newindex");
     lua_newtable (L);
     luaI_add (L, cream_theme_setters);
     lua_pushcclosure (L, luaI_newindex, 1);
     lua_rawset (L, metatable);

     lua_pop (L, 1);
     return 1;
}

/*! @} */
