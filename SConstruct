env = SConscript("godot-cpp/SConstruct")
env.Append(CPPPATH = ["src/"])
sources = Glob("src/*.cpp")

library = env.SharedLibrary("world_project/bin/World{}{}".format(env['suffix'], env['SHLIBSUFFIX']), source = sources)

Default(library)