env = SConscript("godot-cpp/SConstruct")

env["CXXFLAGS"] = [
    flag for flag in env.get("CXXFLAGS", []) if flag != "-fno-gnu-unique"
]

# Ajouter les chemins d'inclusion pour netCDF
env.Append(
    CPPPATH=[
        "src/",
        "/usr/include",  # Pour netcdf.h
        "libs/netcdf-cxx4/cxx4/",  # Pour l'API C++
    ]
)

# Ajouter les bibliothèques
env.Append(LIBS=["netcdf", "netcdf_c++4"])
env.Append(LIBPATH=["/usr/lib", "libs/netcdf-cxx4/build/cxx4"])
env.Append(CXXFLAGS=["-fexceptions"])

env.VariantDir("build", "src", duplicate=0)

sources = Glob("build/*.cpp") + Glob("build/**/*.cpp")

library = env.SharedLibrary(
    "world_project/bin/World{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
