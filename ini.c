#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

#define LUA_FUNC(NAME) static int (NAME)(lua_State *L)
#define REG_NAME "ini"

void fskip_spaces(FILE *fp);

LUA_FUNC(meta_index);

static const luaL_Reg _meta[] = {
    {"__index", meta_index},
    {NULL, NULL}
};

LUA_FUNC(static_new);
LUA_FUNC(static_open);

static const luaL_Reg _static[] = {
    {"test", static_new},
    {"open", static_open},
    {NULL, NULL}
};

__declspec(dllexport) int luaopen_ini(lua_State *L)
{
    lua_settop(L, 0);                       /* top=0 */
    lua_createtable(L, 0, 2);               /* 1 таблица 'static' */
    luaL_setfuncs(L, _static, 0);

    lua_pushvalue(L, LUA_REGISTRYINDEX);    /* 2 */
    lua_pushstring(L, REG_NAME);            /* 3 */
    lua_createtable(L, 0, 1);               /* 4 */
    luaL_setfuncs(L, _meta, 0);
    lua_settable(L, 2);                     /* top=1 */

    lua_settop(L, 1);
    
    return 1;
}

LUA_FUNC(meta_index)
{
    lua_settop(L, 0);
    lua_pushstring(L, "aaa ini");
    return 1;
}

LUA_FUNC(static_new)
{
    lua_settop(L, 0);                       /* top=0 */
    lua_createtable(L, 0, 0);               /* 1 */
    luaL_setmetatable(L, REG_NAME);
    return 1;
}

LUA_FUNC(static_open)   /* ini_table ini.open(file|path)    */
{
    if(lua_gettop(L) < 1 || lua_type(L, 1) == LUA_TNIL)   /* не хватает параметров */
    {
        lua_pushstring(L, "bad argument #1 to 'open' (string or file expected, got no value)");
        lua_error(L);
    }
    else
    {
        FILE *fp;
        int t;

        lua_settop(L, 1);
        t = lua_type(L, 1);
        if(t == LUA_TSTRING)
        {
            fp = fopen(lua_tostring(L, 1), "r");
            if(fp == NULL)
            {
                lua_pushnil(L);                                                 /* 2 */
                lua_pushfstring(L, "%s: Can't open file", lua_tostring(L, 1));  /* 3 */
                return 2;
            }
        }
        else if(t == LUA_TUSERDATA || t == LUA_TLIGHTUSERDATA)
        {
            int has_meta;

            has_meta = lua_getmetatable(L, 1);  /* 2 */  
            if(has_meta)
            {
                const char *name;

                lua_pushstring(L, "__name");    /* 3 */
                lua_gettable(L, 2);             /* 3 */
                name = lua_tostring(L, 3);
                if(!strcmp(name, "FILE*"))
                {
                    luaL_Stream *s;

                    s = (luaL_Stream *)lua_touserdata(L, 1);
                    fp = s->f;
                }
                else
                    goto basic_userdata;
            }
            else
            {
                basic_userdata:
                lua_pushstring(L, "bad argument #1 to 'open' (string or file expected, got no userdata)");
                lua_error(L);
            }
        }
        else
        {
            lua_pushfstring(L, "bad argument #1 to 'open' (string or file expected, got %s)", lua_typename(L, lua_type(L, 1)));
            lua_error(L);
        }

        /* тут нужно обработать файл */
        lua_settop(L, 0);               /* top=0 */
        lua_createtable(L, 0, 0);       /* 1 */
        luaL_setmetatable(L, REG_NAME);

        {
            enum ERRORS { OK=0, NO_SECTION, UNEXP_EOF, STR_TOO_BIG };
            const char *errors[] = {
                NULL,
                "No section defined",
                "Unexpected EOF",
                "String too big",
            };
            char    ch;
            char    buff[1024];
            char    sect[128];
            char    key[128];
            char    val[1024];
            int     in_sect = 0;
            int     error_n = 0;
            int     end = 0;
            int     i;
            int     t0;

            while(!feof(fp))
            {
                fskip_spaces(fp);
                if(feof(fp))
                {
                    error_n = OK;
                    break;
                }

                ch = fgetc(fp);
                #ifdef DEBUG
                printf("'%c'\r\n", ch);
                #endif
                if(ch != '[' && !in_sect)
                {
                    error_n = NO_SECTION;
                    break;
                }
                else if(ch == '[')  /* Установка новой секции */
                {
                    lua_settop(L, 1);   /* top=1 */
                    for(i=0; i < 1023 && !(end = feof(fp)) && (ch = fgetc(fp)) != ']'; i++)
                        buff[i] = ch;
                    buff[i] = 0;
                    if(end)
                    {
                        error_n = UNEXP_EOF;
                        break;
                    }

                    if(i > 127)
                    {
                        error_n = STR_TOO_BIG;
                        break;
                    }

                    strcpy(sect, buff);
                    in_sect = 1;

                    lua_pushstring(L, sect);    /* 2 */
                    lua_pushvalue(L, 2);        /* 3 */
                    t0 = lua_rawget(L, 1);     /* 3 */
                    /*
                        1 out
                        2 section name
                        3 section table | nil
                    */
                    if(t0 == LUA_TNIL)
                    {
                        lua_pop(L, 1);              /* top=2 */
                        lua_createtable(L, 0, 0);   /* 3 */
                        lua_pushvalue(L, 3);        /* 4 */
                        lua_insert(L, 2);
                        /*
                            1 out
                            2 section table
                            3 section name  -\
                            4 section table --- добавить в 'out'
                        */
                        lua_settable(L, 1); /* top=2 */
                    }
                    else if(t0 == LUA_TTABLE)
                    {
                        lua_replace(L, 2);
                        /*
                            1 out
                            2 section table
                            3 section name
                        */
                        lua_pop(L, 1);  /* top=2 */
                    }
                    /*
                        1 out
                        2 section table - уже в 'out'
                    */
                }
                else    /* Обработка пар K-V */
                {
                    lua_settop(L, 2);   /* top=2 */
                    fskip_spaces(fp);
                    for(i=0; i < 1023 && !(end = feof(fp))
                        && (ch = fgetc(fp)) != ' ' && ch != '='
                        && ch != '\n'; i++)
                        buff[i] = ch;
                    buff[i] = 0;

                    if(end)
                    {
                        error_n = UNEXP_EOF;
                        break;
                    }

                    if(i > 127)
                    {
                        error_n = STR_TOO_BIG;
                        break;
                    }

                    strcpy(key, buff);

                    fskip_spaces(fp);
                    for(i=0; i < 1023 && !(end = feof(fp))
                        && (ch = fgetc(fp)) != '\n'; i++)
                        buff[i] = ch;
                    buff[i] = 0;

                    if(end)
                    {
                        error_n = UNEXP_EOF;
                        break;
                    }

                    strcpy(val, buff);

                    lua_pushstring(L, key); /* 3 */
                    lua_pushstring(L, val); /* 4 */
                    lua_settable(L, 2);     /* top=2 */
                }
            }

            if(t == LUA_TSTRING)
            {
                #ifdef DEBUG
                puts("FILE CLOSED");
                #endif
                fclose(fp);
            }
            lua_settop(L, 1);
            if(error_n)
            {
                lua_pushnil(L);
                lua_pushstring(L, errors[error_n]);
                return 2;
            }
            else
            {
                return 1;
            }
        }
    }
}

void fskip_spaces(FILE *fp)
{
    int end;
    #ifdef DEBUG
    char ch;
    while(isspace((ch = fgetc(fp))) && !(end = feof(fp)))
        printf("'%c'\r\n", ch);
    printf("'%c'\r\n", ch);
    #else
    while(isspace(fgetc(fp)) && !(end = feof(fp))) {}
    #endif
    if(!end) fseek(fp, -1, SEEK_CUR);
}
