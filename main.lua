local ini = require("ini")

file = io.open("a.ini", "r")
local a, r, n = ini.open("a.ini")

if a then
    for k, v in pairs(a) do
        print('[' .. k .. ']')
        for k, v in pairs(v) do
            print(k .. ': ' .. v)
        end
        print("")
    end
else
    print("Error#" .. tostring(n) .. ": " .. r)
end

file:close()
