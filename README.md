# Lua-ini
Ini parserfor Lua

# Build
Use `makefile` to build lib for Windows.
Makefile for Linux will be later.
For mac... May be later.

## Configs
Some settings are located in file `/ini-cfg.h`. There defined some macroses and array of strings.

<table>

|Name|Type|Meaning|Default|
|-|:-:|-|-|
|`INI_SECTION_NAME_START`|macro/char|Defines symbol wich opens section's header|`('[')`|
|`INI_SECTION_NAME_STOP`|macro/char|Defines symbol wich closes section's header|`(']')`|
|`INI_SEPORATE`|macro/char|Defines seporator betwin keys and values|`('=')`|
|`INI_COMMENT`|macro/char|Defines symbol wich marks all following text on this line as comment|`('#')`|
|`INI_SCREEN`|macro/char|Defines symbol wich used as screen character. Symbol after screen will be copied to key or value without parsing. With that symbol you can insert symbols as `=#\` in keys and values|`('\\')`|
|`INI_ERRORS`|enum|Defines error codes. You can use it to compare with error code, received from `ini.open`, insted of compairing reason string. Do not change names: will cause compilation errors.|
|`ini_errors_text`|array of C-strings|Defines error messages(used as reason). Corresponds with `INI_ERRORS`|

## Important
Remove macro `DEBUG` to hide debug output.

# From Lua
Some exapmles:
```Lua
local ini = require("ini")  -- Loads library

local a, reason, code = ini.open("a.ini") -- Open and parse file
a       -- Ini object(table) or nil, if error
reason  -- Reason, why lib can't open file or nil, if no errors
code    -- Error code(see enum `INI_ERRORS`) or nil, if no errors

local file = io.open("a.ini", "r")  -- Does same thing,
local b = ini.open(file)            -- but uses file, open and managed by Lua
file:close()                        -- You still have to close your file

a:save()        -- Save object to file (path stored in field '__path')
ini.save(a)     -- same
a:save("b.ini") -- Save object to file 'b.ini' and change field '__path'

a = b = nil -- No special destructor needed
```

Sections are stored directly in table with keys equals to section's name, so you can't use section \[__path\].
Keys and values are stored directly in section's tables.

## Important
Section's names, keys and values can be saved only as string

# ini expamples
Some available structures may be not standard

```ini
# This is basic coment line wich is skiped by parser
[a]         # Begining of new section or continuing of already existing with same name
a=42        # Basic key-value pair were `a` is key and `42` is value

[a#1]       # Special symbols aren't working in section's names
a\#2=b\#1   # Here key is `a#1` and value is `b#1`

[a]         # Overriding values in section `a`
a=22        # Parser will set new value for `a` without raising any errors
```

# Other files
- `lua/` - Lua headersand binaries for building library
- `a.ini` - example ini file
- `main.lua` - test script

# Bugs
- 'Mistery' empty strings in output files(at least, everything is working)
- Fields saved in random order
- Section `__path`(Not alowed) raises Lua-error. Be careful
- <b>Memmory allocated but not cleaned</b>

<hr>
Sory for lots of small and almost useles commits and my English