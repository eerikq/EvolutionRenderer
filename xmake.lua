---@diagnostic disable
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")

set_languages("c++20")
set_toolchains("clang")

target("EvolutionRenderer")
set_kind("binary")
add_files("src/*.cpp")
