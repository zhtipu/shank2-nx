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

// Mounts the NSP's RomFS as "shank2rom:" once. It stays mounted only when it holds the game pack or the shader
// package; for an NRO started from the homebrew menu (whose process RomFS is some other title's or none) it is
// unmounted again.
bool EnsureRomfsMounted() {
#if defined(__SWITCH__)
  static bool tried = false;
  static bool mounted = false;
  if (tried) return mounted;
  tried = true;
  FsFileSystem fs;
  if (R_FAILED(fsOpenDataFileSystemByCurrentProcess(&fs))) return false;
  if (fsdevMountDevice("shank2rom", fs) == -1) return false;  // fsdev closes fs on failure
  struct stat st;
  mounted = (stat("shank2rom:/game_root.nxpack", &st) == 0 && stat("shank2rom:/game_root.nxidx", &st) == 0) ||
            stat("shank2rom:/deko3d_glsl.bin", &st) == 0;
  if (!mounted) fsdevUnmountDevice("shank2rom");
  return mounted;
#else
  return false;
#endif
}

// The game data in the running NSP's RomFS, as "shank2rom:/game_root", or empty when this is not the NSP (or the NSP
// has no game pack).
std::string MountNspGameData() {
#if defined(__SWITCH__)
  static std::string root;
  static bool tried = false;
  if (tried) return root;
  tried = true;
  if (!EnsureRomfsMounted()) return root;
  struct stat st;
  if (stat("shank2rom:/game_root.nxpack", &st) == 0 && stat("shank2rom:/game_root.nxidx", &st) == 0)
    root = "shank2rom:/game_root";
  return root;
#else
  return {};
#endif
}

}  // namespace shank2

// "shank2rom:/<name>" when the running NSP's RomFS has that file, else null (used by the shader loader).
extern "C" const char *rex_nsp_romfs_file(const char *name) {
#if defined(__SWITCH__)
  static std::string path;
  if (!shank2::EnsureRomfsMounted()) return nullptr;
  path = std::string("shank2rom:/") + name;
  struct stat st;
  return stat(path.c_str(), &st) == 0 ? path.c_str() : nullptr;
#else
  (void)name;
  return nullptr;
#endif
}
