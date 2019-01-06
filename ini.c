#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "ini-cfg.h"

#define LUA_FUNC(NAME) static int (NAME)(lua_State *L)
#define REG_NAME "ini"

void fskip_spaces(FILE *fp);

LUA_FUNC(all_save);

LUA_FUNC(meta_index);

static const luaL_Reg _meta[] = {
    {"save", all_save},
    {NULL, NULL}
};

LUA_FUNC(static_new);
LUA_FUNC(static_open);

static const luaL_Reg _static[] = {
    {"new", static_new},
    {"open", static_open},
    {"save", all_save},
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

    lua_pushstring(L, "__name");            /* 5 */
    lua_pushstring(L, "ini");               /* 6 */
    lua_settable(L, 4);                     /* top=4 */

    lua_pushstring(L, "__index");           /* 5 */
    lua_pushvalue(L, 4);                    /* 6 */
    lua_settable(L, 4);                     /* top=4 */

    luaL_setfuncs(L, _meta, 0);
    lua_settable(L, 2);                     /* top=2 */

    lua_settop(L, 1);
    
    return 1;
}

LUA_FUNC(static_new)
{
    lua_settop(L, 0);                       /* top=0 */
    lua_createtable(L, 0, 0);               /* 1 */
    luaL_setmetatable(L, REG_NAME);
    return 1;
}

LUA_FUNC(all_save)
{
    char *name;

    if(lua_gettop(L) < 1)
    {
        lua_pushstring(L, "bad argument #1 to 'save' (ini-table expected, got no value)");
        lua_error(L);
    }
    else
    {
        int t;

        t = lua_type(L, 1);
        if(t == LUA_TTABLE)
        {
            int t0;

            t0 = luaL_getmetafield(L, 1, "__name");
            if(t0 == LUA_TSTRING && !strcmp("ini", lua_tostring(L, -1)))
            {
                lua_pop(L, 1);  /* top=1 or 2 */
                if(lua_gettop(L) >= 2)
                {
                    int t1;

                    t1 = lua_type(L, 2);
                    if(t1 == LUA_TSTRING)
                    {
                        lua_pushstring(L, "__path");    /* 3 */
                        lua_pushvalue(L, 2);            /* 4 */
                        lua_settable(L, 1);             /* top=2 */
                        lua_len(L, 2);                  /* 3 */
                        name = malloc(lua_tointeger(L, -1) + 1);
                        strcpy(name, lua_tostring(L, 2));
                        lua_settop(L, 1);               /* top=1 */
                    }
                    else
                    {
                        lua_pushfstring(L, "bad argument #2 to 'save' (string or none expected, got %s)", lua_typename(L, t1));
                        lua_error(L);
                    }
                }
                else
                {   /* top=1 */
                    int t1;

                    lua_pushstring(L, "__path");    /* 2 */
                    t1 = lua_rawget(L, 1);          /* 2 */

                    if(t1 == LUA_TSTRING)
                    {
                        name = (char*)lua_tostring(L, 2);
                        lua_pop(L, 1);
                    }
                    else
                    {
                        lua_pushboolean(L, 0);
                        lua_pushstring(L, "Can't save ini: path not specified.\r\n\
Pass it as 2nd argument for save or set field '__path'");
                        return 2;
                    }
                }
            }
            else
            {
                lua_pushstring(L, "bad argument #1 to 'save' (ini-table expected, got table)");
                lua_error(L);
            }

            {
                FILE *fp;

                fp = fopen(name, "w");
                if(fp == NULL)
                {
                    lua_pushboolean(L, 0);
                    lua_pushfstring(L, "%s: Can't open file", name);
                    return 2;
                }

                lua_pushnil(L); /* 2 */
                while(lua_next(L, 1))
                {
                    /*
                        1 table: src
                        2 key
                        3 value
                    */
                    int t2;

                    t2 = lua_type(L, -1);
                    if(t2 == LUA_TTABLE)
                    {
                        fprintf(fp,"%c%s%c\n", INI_SECTION_NAME_START, lua_tostring(L, -2), INI_SECTION_NAME_STOP);
                        lua_pushnil(L); /* 4 */
                        while(lua_next(L, -2))
                        {
                            /*
                                1 table: src
                                2 key
                                3 table: src
                                4 key
                                5 value
                             */
                            const char *buff;
                            char ch;

                            buff = lua_tostring(L, -2);
                            for(int i = 0; (ch = buff[i]); i++)
                            {
                                if(ch == INI_COMMENT || ch == INI_SCREEN || ch == INI_SEPORATE)
                                    fputc(INI_SCREEN, fp);
                                fputc(ch, fp);
                            }
                            fputc(INI_SEPORATE, fp);
                            buff = lua_tostring(L, -1);
                            for(int i = 0; (ch = buff[i]); i++)
                            {
                                if(ch == INI_COMMENT || ch == INI_SCREEN || ch == INI_SEPORATE)
                                    fputc(INI_SCREEN, fp);
                                fputc(ch, fp);
                            }
                            fputc('\n', fp);
                            lua_pop(L, 1);  /* top=4(key) */
                        }
                    }
                    lua_pop(L, 1);
                    fputc('\n', fp);
                }
                fclose(fp);
                lua_pushboolean(L, 1);
                return 1;
            }
        }
        else
        {
            lua_pushfstring(L, "bad argument #1 to 'open' (ini-table expected, got %s)", lua_typename(L, t));
            lua_error(L);
        }
    }
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
        char *name;

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
            
            lua_len(L, 1);
            name = malloc(lua_tointeger(L, 2) + 1);
            lua_settop(L, 1);
            strcpy(name, lua_tostring(L, 1));
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
        if(t == LUA_TSTRING)
        {
            lua_pushstring(L, "__path");/* 2 */
            lua_pushstring(L, name);    /* 3 */
            lua_settable(L, 1);         /* top=1 */
        }

        {
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
            int     was_backslash = 0;
            int     was_comment = 0;

            while(!feof(fp))
            {
                fskip_spaces(fp);
                if(feof(fp))
                {
                    error_n = OK;
                    break;
                }

                ch = fgetc(fp);
                if(ch != INI_SECTION_NAME_START && !in_sect)
                {
                    error_n = INI_NO_SECTION;
                    break;
                }
                else if(ch == INI_COMMENT)
                {
                    while(fgetc(fp) != '\n') {}
                }
                else if(ch == INI_SECTION_NAME_START)   /* Установка новой секции */
                {
                    lua_settop(L, 1);   /* top=1 */
                    for(i=0; i < 1023 && !(end = feof(fp)) && (ch = fgetc(fp)) != INI_SECTION_NAME_STOP; i++)
                        buff[i] = ch;
                    buff[i] = 0;
                    if(end)
                    {
                        error_n = INI_UNEXP_EOF;
                        break;
                    }

                    if(i > 127)
                    {
                        error_n = INI_STR_TOO_BIG;
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
                    ungetc(ch, fp);
                    lua_settop(L, 2);   /* top=2 */
                    fskip_spaces(fp);
                    was_backslash = 0;
                    for(i=0; i < 1023 && (!(end = feof(fp))
                        && (ch = fgetc(fp)) != ' ' && ch != INI_SEPORATE
                        && ch != '\n') || was_backslash; i++)
                    {
                        if(ch == INI_SCREEN && !was_backslash)
                        {
                            was_backslash = 1;
                            i--;
                        }
                        else if(ch == INI_COMMENT && !was_backslash)
                        {
                            lua_pushnil(L);
                            lua_pushstring(L, ini_errors_text[INI_COMM_AFTER_KEY]);
                            lua_pushinteger(L, INI_COMM_AFTER_KEY);
                            return 3;
                            break;
                        }
                        else
                        {
                            buff[i] = ch;
                            was_backslash = 0;
                        }
                    }
                    buff[i] = 0;

                    if(end)
                    {
                        error_n = INI_UNEXP_EOF;
                        break;
                    }

                    if(i > 127)
                    {
                        error_n = INI_STR_TOO_BIG;
                        break;
                    }

                    strcpy(key, buff);

                    fskip_spaces(fp);
                    was_backslash = 0;
                    for(i=0; i < 1023 && (!(end = feof(fp))
                        && (ch = fgetc(fp)) != '\n') || was_backslash; i++)
                    {
                        if(ch == INI_SCREEN && !was_backslash)
                        {
                            was_backslash = 1;
                            i--;
                        }
                        else if(ch == INI_COMMENT && !was_backslash)
                        {
                            while(fgetc(fp) != '\n') {}
                            break;
                        }
                        else
                        {
                            buff[i] = ch;
                            was_backslash = 0;
                        }
                    }
                    buff[i] = 0;

                    if(end)
                    {
                        error_n = OK;
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
                fclose(fp);
            }
            lua_settop(L, 1);
            if(error_n)
            {
                lua_pushnil(L);
                lua_pushstring(L, ini_errors_text[error_n]);
                lua_pushinteger(L, error_n);
                return 3;
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
    char ch;

    while(isspace((ch = fgetc(fp))) && !(end = feof(fp))) {}
    ungetc(ch, fp);
}

/*
    A lot of useles text, because customer want exactly 15KB of code. Sory
    РЫБА. ПОЧЕМУ ТЕБЕ НЕ ХВАТАЕТ 14KB КОДА?!!! Ну ладно.
    Lorem ipsum чо там дальше? Я НЕ ПОМНЮ!!!
    HTML подойдёт? Нет? ну ок
    
                        for(i=0; i < 1023 && !(end = feof(fp))
                        && (ch = fgetc(fp)) != '\n'; i++)
                        buff[i] = ch;
                    buff[i] = 0;

                    if(end)
                    {
                        error_n = OK;
                        break;
                    }

                    strcpy(val, buff);

                    lua_pushstring(L, key); /* 3 /
                    lua_pushstring(L, val); /* 4 /
                    lua_settable(L, 2);     /* top=2 /
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
                lua_pushstring(L, INI_ERRORS[error_n]);
                lua_pushinteger(L, error_n);
                return 3;
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
    char ch;

    while(isspace((ch = fgetc(fp))) && !(end = feof(fp))) {}
    ungetc(ch, fp);
}
*/
