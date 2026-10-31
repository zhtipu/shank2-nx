// Shank 2 installed as an NSP (nsp/): the loader starts shank2.nro from the NSP's RomFS, and the game data
// (game_root.nxpack + .nxidx) is in that RomFS too. The RomFS is mounted as a file system device, so the asset pack's
// Switch storage (pack_storage_switch.cpp: fsdevTranslatePath, fsFsOpenFile) reads it the same way as on the SD card.
// Kept apart from the app header because switch.h brings u8/Result into the global namespace.
#include <string>

#if defined(__SWITCH__)
#include <sys/stat.h>

#include <switch.h>
#endif

namespace shank2 {

// The game data in the running NSP's RomFS, as "shank2rom:/game_root", or empty when this is not the NSP (the NRO
// started from the homebrew menu, whose process RomFS is some other title's or none).
std::string MountNspGameData() {
#if defined(__SWITCH__)
  static std::string root;
  static bool tried = false;
  if (tried) return root;
  tried = true;
  FsFileSystem fs;
  if (R_FAILED(fsOpenDataFileSystemByCurrentProcess(&fs))) return root;
  if (fsdevMountDevice("shank2rom", fs) == -1) return root;  // fsdev closes fs on failure
  struct stat st;
  if (stat("shank2rom:/game_root.nxpack", &st) == 0 && stat("shank2rom:/game_root.nxidx", &st) == 0) {
    root = "shank2rom:/game_root";
  } else {
    fsdevUnmountDevice("shank2rom");
  }
  return root;
#else
  return {};
#endif
}

}  // namespace shank2
