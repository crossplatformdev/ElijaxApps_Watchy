Import("env")

map_path = env.subst("$BUILD_DIR/firmware.map")
env.Append(LINKFLAGS=[f"-Wl,-Map,{map_path}", "-Wl,--cref"])