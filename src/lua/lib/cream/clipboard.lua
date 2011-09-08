-- Clipboard
-- @author David Delassus &lt;david.jose.delassus@gmail.com&gt;

local setmetatable = setmetatable
local capi =
{
     clipboard = Clipboard
}

module ("cream.clipboard")

primary = capi.clipboard.new ("PRIMARY")
default = capi.clipboard.new ("CLIPBOARD")

setmetatable (_M, { __call = function (_, ...) return capi.clipboard.new (...) end })
