---@diagnostic disable
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")

-- toolchain
set_languages("c++20")
set_toolchains("clang")

-- libraries
add_requires("volk 1.4.335+0")
add_requires("vulkan-headers 1.4.335+0")
add_requires("vulkan-memory-allocator 3.3.0")

add_requires("glm 1.0.3")
add_requires("fmt 12.2.0")
add_requires("stb 2026.03.18")

add_requires("libsdl3 3.4.12")
add_requires("imgui v1.92.7-docking", {
    configs = {
        sdl3 = true,
        sdl3_renderer = true,
        vulkan = true,
        volk = true,
    },
})

target("evolution-renderer")
    set_kind("binary")
    add_files("src/**.cpp")
    add_includedirs("src")

    add_cxflags("-ferror-limit=0", { tools = { "clang", "clangxx" } })
    add_defines("SDL_MAIN_HANDLED")
    add_defines("VK_NO_PROTOTYPES")

    add_packages("volk")
    add_packages("vulkan-headers")
    add_packages("vulkan-memory-allocator")

    add_packages("glm")
    add_packages("fmt")
    add_packages("stb")

    add_packages("libsdl3")
    add_packages("imgui")
