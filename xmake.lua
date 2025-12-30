add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "build" })

set_warnings("all")
set_languages("c++17")
if is_mode("debug") then
	set_optimize("none")
	set_symbols("debug")
else
	set_optimize("faster")
	set_symbols("hidden")
end

set_policy("package.install_locally", true)

add_requires("gtest", {})

target("structs")
set_kind("static")
add_files("src/*.cpp")
target_end()

for _, file in ipairs(os.files("tests/test_*.cpp")) do
	local name = path.basename(file)
	target(name)
	set_kind("binary")
	set_default(false)
	add_files(file)
	add_tests("default")
	add_deps("structs")
	add_packages("gtest")
	target_end()
end
