package = "dr.msgpuck"
version = "0.1-2"
source = {
   url = "git+https://github.com/dr-co/lua-msgpuck",
   branch = "master"
}
description = {
   summary = "LuaC bindings for rtsisyk/msgpuck",
   homepage = "https://github.com/dr-co/lua-msgpuck",
   license = "BSD"
}
dependencies = {
   "lua >= 5.1"
}
build = {
   type = "builtin",
   modules = {
      ["dr.msgpuck"] = {
         sources = {
            "src/dr-msgpuck.c",
            -- "src/buffer.h",
            -- "src/msgpack.h",

            "src/hints.c",
            "src/msgpuck.c",
            -- "src/msgpuck.h",

         }
      }
   }
}

