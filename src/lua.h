#ifndef __LUA_H
#define __LUA_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>

/*!
 * \defgroup lua Lua context
 * Functions to manipulate the Lua context.
 *
 * @{
 */

static inline void luaL_checktable (lua_State *L, int idx)
{
     luaL_checktype (L, idx, LUA_TTABLE);
}

static inline gboolean luaL_checkboolean (lua_State *L, int idx)
{
     luaL_checktype (L, idx, LUA_TBOOLEAN);
     return lua_toboolean (L, idx);
}

/*!
 * \def LUA_TMODULE
 * Userdata type associated to #CreamModule.
 */
#define LUA_TMODULE    "CreamModule"

/*!
 * \def LUA_TCLIPBOARD
 * Userdata type associated to \class{GtkClipboard}.
 */
#define LUA_TCLIPBOARD "Clipboard"

/*!
 * \def LUA_TREGEX
 * Userdata type associated to \class{GRegex}.
 */
#define LUA_TREGEX     "Regex"

/*!
 * \def LUA_TWEBVIEW
 * Userdata type associated to #WebView.
 */
#define LUA_TWEBVIEW   "WebView"

/*!
 * \def LUA_TTHEME
 * Userdata type associated to #theme_t.
 */
#define LUA_TTHEME     "theme"


#define LUA_TNOTEBOOK    "Notebook"
#define LUA_TVIEWAREA    "ViewArea"
#define LUA_TSTATUSBAR   "Statusbar"
#define LUA_TPROMPTBOX   "Promptbox"

gboolean lua_ctx_init (GError **err);
gboolean lua_ctx_parse (const char *file, GError **err);
void lua_ctx_close (void);

extern void lua_pushwebview (lua_State *L, WebView *w);

/* Used for __index and __newindex */

typedef int (*luaI_func) (lua_State *L, gpointer v);

/*!
 * \struct luaI_reg
 * Member info for get and set handlers
 */
typedef const struct
{
     const gchar *name;       /*!< member name. */
     luaI_func func;          /*!< get or set function for type of member. */
     size_t offset;           /*!< offset of member with its type. */
} luaI_reg_pre;

typedef luaI_reg_pre * luaI_reg;

void luaI_add (lua_State *L, luaI_reg l);
int luaI_index (lua_State *L);
int luaI_newindex (lua_State *L);

int luaI_getint (lua_State *L, gpointer v);
int luaI_setint (lua_State *L, gpointer v);

int luaI_getbool (lua_State *L, gpointer v);
int luaI_setbool (lua_State *L, gpointer v);

int luaI_getstring (lua_State *L, gpointer v);
int luaI_setstring (lua_State *L, gpointer v);

/*! @} */

#endif /* __LUA_H */
