#!/usr/bin/env python
import os
import subprocess
import shutil

from SCons.Script.SConscript import SConsEnvironment

env: SConsEnvironment = SConscript("godot-cpp/SConstruct")

# --- Build RNNoise (vendored via the noise-suppression-for-voice submodule, which
# --- wraps xiph/rnnoise with a CMakeLists.txt so it cross-compiles cleanly through
# --- CMake instead of upstream rnnoise's autotools build). ---

rnnoise_src_dir = "noise-suppression-for-voice/external/rnnoise"
rnnoise_build_dir = os.path.join(rnnoise_src_dir, "cmake_build_out", "{}_{}".format(env["platform"], env["arch"]))
rnnoise_config = "RelWithDebInfo" if env["dev_build"] else "Release"
rnnoise_lib_dir = os.path.join(rnnoise_build_dir, rnnoise_config)


def build_rnnoise(target, source, env):
    extra_flags = []
    if env["platform"] in ("linux", "macos"):
        extra_flags += ["-DCMAKE_POSITION_INDEPENDENT_CODE=ON"]
    if env["platform"] == "macos":
        extra_flags += ["-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64", "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15"]

    generator_args = []
    if env["platform"] in ("linux", "macos"):
        generator_args = ["-G", "Ninja Multi-Config"]
    # On Windows, no -G is passed: CMake picks the installed Visual Studio
    # generator, which is multi-config like Ninja Multi-Config, so the
    # RelWithDebInfo/Release subdirectory layout stays consistent everywhere.

    config_args = ["cmake", "-S", rnnoise_src_dir, "-B", rnnoise_build_dir] + generator_args + extra_flags
    print("Configuring RNNoise:", " ".join(config_args))
    subprocess.run(config_args, check=True)

    build_args = ["cmake", "--build", rnnoise_build_dir, "--config", rnnoise_config]
    print("Building RNNoise:", " ".join(build_args))
    subprocess.run(build_args, check=True)


def clean_rnnoise(target, source, env):
    if os.path.isdir(rnnoise_build_dir):
        shutil.rmtree(rnnoise_build_dir)


rnnoise_stamp = env.Command(os.path.join(rnnoise_build_dir, "rnnoise_stamp"), [], build_rnnoise)
env.AlwaysBuild(rnnoise_stamp)
env.Command("clean_rnnoise", [], clean_rnnoise)

env.Append(
    CPPPATH=[os.path.join(rnnoise_src_dir, "include")],
    LIBS=["RnNoise"],
    LIBPATH=[rnnoise_lib_dir],
)

# --- Build the extension itself ---

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

addon_dir = "example/addons/godot_rnnoise/bin"

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{}/libgodot_rnnoise.{}.{}.framework/libgodot_rnnoise.{}.{}".format(
            addon_dir, env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/libgodot_rnnoise{}{}".format(addon_dir, env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

env.Depends(library, rnnoise_stamp)
env.NoCache(library)
Default(library)
