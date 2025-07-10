-- Sandbox Loader

add_deps("cabin")
set_rundir("$(projectdir)/sandbox")

for _, subdir in ipairs(os.dirs(os.scriptdir() .. "/*")) do
    if os.exists(subdir .. "/xmake.lua") then
        includes(subdir)
    end
end