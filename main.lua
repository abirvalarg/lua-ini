local ini = require("ini")

local a, r, n = ini.open("a.ini")

if a then
    a.c = a.b
    a.c.c = "c"

    local out, s, r
    s, r = a:save()
    if s then
        out = "Saved"
    else
        out = "Fail"
    end
    print(out)
else
    print("Error#" .. tostring(n) .. ": " .. r)
end
