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
-- add_requires("slang 2026.16") -- don't think i need this?

add_requires("tinyobjloader")
-- add_requires("cgltf 1.15") -- implement after obj importing works
-- add_requires("assimp 6.0.5") -- assimp seems cool but i think cgltf does things in a way i like more

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

target("evolution_renderer")
    set_kind("binary")
    add_files("src/**.cpp")
    add_includedirs("src")

    -- defines
    add_cxflags("-ferror-limit=0", { tools = { "clang", "clangxx" } })
    add_defines("SDL_MAIN_HANDLED")
    add_defines("VK_NO_PROTOTYPES")

    -- build scripts
    after_build(function(target)
        import("core.project.task")

        task.run("copy_assets")
        task.run("compile_shaders")
    end)

    -- packages
    add_packages("volk")
    add_packages("vulkan-headers")
    add_packages("vulkan-memory-allocator")

    add_packages("tinyobjloader")

    add_packages("glm")
    add_packages("fmt")
    add_packages("stb")

    add_packages("libsdl3")
    add_packages("imgui")
target_end()

task("copy_assets")
    on_run(function()
        local asset_path = path.join(os.projectdir(), "assets")
        local build_path = path.join(os.projectdir(), get_config("builddir"), os.host(), os.arch(), get_config("mode"))

        os.cp(asset_path, build_path)
        print("Moved assets from", asset_path, "to", build_path)
    end)
task_end()

task("compile_shaders")
    on_run(function()
        local shader_name = "shader.slang"
        local shader_output = "slang.spv"

        local asset_path = path.join(os.projectdir(), "shaders", shader_name)
        local build_path = path.join(os.projectdir(), get_config("builddir"), os.host(), os.arch(), get_config("mode"), "shaders")
        local output_file = path.join(build_path, shader_output)

        if not os.exists(build_path) then
            os.mkdir(build_path)
        end

        os.runv("slangc", {
            asset_path,
            "-target", "spirv",
            "-profile", "spirv_1_4",
            "-emit-spirv-directly", "-fvk-use-entrypoint-name",
            "-entry", "vertMain",
            "-entry", "fragMain",
            "-o", output_file,
        })

        print("Compiled", asset_path, "to", output_file)
    end)
task_end()
