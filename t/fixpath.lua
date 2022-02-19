package.path = string.format('%s;%s', '.rocks/share/lua/5.1/?.lua', package.path)
package.path = string.format('%s;%s', 'build/?.lua', package.path)
package.cpath = string.format('%s;%s', 'build/?.so', package.cpath)


package.cpath = string.format('%s;%s', '.rocks/lib/lua/5.1/?.so', package.cpath)

return {}
