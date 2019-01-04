# Lua-ini
Ini parserfor Lua

# Build
Use `makefile` to build lib for Windows.
Makefile for Linux will be later.
For mac... May be later.

## Important
Remove macro `DEBUG` to hide debug output.

# From Lua
Some exapmles:
```Lua
local ini = require("ini")  -- Loads library

local a = ini.open("a.ini") -- Open and parse file

local file = io.open("a.ini", "r")  -- Does same thing,
local b = ini.open(file)            -- but uses file, open and controlled by Lua
file:close()                        -- ini.open will not close the file from Lua

a:save()        -- Save object to file (path stored in field '__path')
ini.save(a)     -- same
a:save("b.ini") -- Save object to file 'b.ini' and change field '__path'

a = b = nil -- No special destructor needed
```

Sections are stored directly in table with keys equals to section's name, so you can't use section \[__path\].
Keys and values are stored directly in section's tables.

## Important
Section's names, keys and values can be saved only as string

# Other files
- `lua/` - Lua headersand binaries for building library
- `a.ini` - example ini file
- `main.lua` - test script

# Other information and bugs
Will be fixed or added on next week. Or maybe later...