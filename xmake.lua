---@diagnostic disable
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")

-- toolchain
set_languages("c++20")
set_toolchains("clang")

-- libraries
add_requires("libsdl3")
--add_requires("imgui v1.92.5-docking", {
--    configs = {
--        sdl3 = true,
--        sdl3_renderer = true,
--    },
--})

target("EvolutionRenderer")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("libsdl3")
