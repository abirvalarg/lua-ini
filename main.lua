local ini = require("ini")

file = io.open("a.ini", "r")
local a, r = ini.open("a.ini")

if a then
    for k, v in pairs(a) do
        print("(" .. type(k) .. ")" .. tostring(k) .. " = (" .. type(v) .. ")" .. tostring(v))
    end
else
    print(r)
end

file:close()
