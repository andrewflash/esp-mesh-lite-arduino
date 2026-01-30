"""
PlatformIO build script for ESP-Mesh-Lite Arduino Library
Links the correct pre-compiled library based on target chip.
"""

Import("env")
import os

# Get library root directory
try:
    lib_path = os.path.dirname(os.path.realpath(__file__))
except NameError:
    # __file__ may be undefined in SCons context; fall back to library source dir.
    lib_path = env.get("LIBSOURCE_DIR") or env.Dir(".").abspath

# Detect target chip
try:
    mcu = env.BoardConfig().get("build.mcu", "esp32").lower()
except:
    mcu = "esp32"

# Supported chips
chips = ["esp32", "esp32s2", "esp32s3", "esp32c2", "esp32c3", "esp32c6"]
chip = mcu if mcu in chips else "esp32"

# Link pre-compiled library
lib_file = os.path.join(lib_path, "lib", chip, "libesp_mesh_lite.a")
if os.path.exists(lib_file):
    env.Append(LIBS=[env.File(lib_file)])
    print(f"[ESP-Mesh-Lite] Linking: lib/{chip}/libesp_mesh_lite.a")
else:
    print(f"[ESP-Mesh-Lite] ERROR: Library not found for {chip}")

# Required defines
env.Append(CPPDEFINES=[
    ("CONFIG_MESH_LITE_ENABLE", 1),
    ("CONFIG_BRIDGE_EXTERNAL_NETIF_STATION", 1),
    ("CONFIG_BRIDGE_DATA_FORWARDING_NETIF_SOFTAP", 1),
])
