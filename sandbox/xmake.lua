-- Sandbox Loader

add_deps("cabin")

for _, subdir in ipairs(os.dirs(os.scriptdir() .. "/*")) do
    if os.exists(subdir .. "/xmake.lua") then
        includes(subdir)
    end
end