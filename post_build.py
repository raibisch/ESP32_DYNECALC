
from shutil import copyfile
Import("env","projenv") # type: ignore

FIRMWARE_BIN = "$BUILD_DIR/${PROGNAME}.bin"
SPIFFS_BIN =  "$BUILD_DIR/spiffs.bin"
LITTLEFS_BIN= "$BUILD_DIR/littlefs.bin"

def copy_firmware(source, target, env):
     file1 = target[0].get_abspath()
     file2 = env["PROJECT_DIR"] + "/upload/firmware.bin"
     print("--> copy_firmware: " + file1)
     print("--> to           : " + file2)
     copyfile(file1, file2)
     print("--> ***Firmware build OK!*** :-)")
    
   
def copy_spiffs(source, target, env):
    file1 = target[0].get_abspath()
    file2 = env["PROJECT_DIR"] + "/upload/spiffs.bin"
    print("--> copy_spiffs: " + file1)
    print("--> to         : " + file2)
    copyfile(file1, file2)
    print("--> *** SPIFF build OK*** :-)")

env.AddPostAction(FIRMWARE_BIN, copy_firmware)
env.AddPostAction(LITTLEFS_BIN, copy_spiffs)
