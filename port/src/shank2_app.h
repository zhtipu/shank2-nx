// shank2 - ReXGlue Recompiled Project
//
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include <rex/filesystem.h>
#include <rex/logging.h>
#include <rex/platform.h>
#include <rex/rex_app.h>

// Renut's Direct3D-level renderer (sa2-nx's copy of reNut, src/renut_engine/gpu), chosen at boot with
// sa2_renderer = "renut". Without it the app uses ReXGlue's generic Xenos GPU emulation.
#if SHANK2_RENUT_RENDERER
#include "renut_engine/gpu/banjo_renut_select.h"
#include "renut_engine/gpu/overlay/app_integration.h"
#include "renut_engine/gpu/banjo_renut_capture.h"
#include "sa2_capture.h"
#endif

namespace shank2 {
// shank2_nsp.cpp: the game data in the running NSP's RomFS ("shank2rom:/game_root"), or empty outside the NSP.
std::string MountNspGameData();
}  // namespace shank2

class Shank2App : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

#if REX_PLATFORM_SWITCH
  // The console build's fixed settings, applied before shank2.toml (which may override any of them). The NSP needs no
  // settings file on the SD card: the native renderer on deko3d and the full game are the defaults.
  std::string_view BuiltInConfig() const override {
    return "license_mask = 1\n"
           "sa2_renderer = \"renut\"\n"
           "renut_gpu_backend = \"deko3d\"\n"
           "sa2_vsync_relaxed = true\n";
  }
#endif

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<Shank2App>(new Shank2App(ctx, "shank2",
        PPCImageConfig));
  }

  // Without --game_data_root, use a game_root/ folder next to the executable (on the Switch:
  // sdmc:/switch/shank2-nx/game_root). The settings file is read after this hook runs, so the game folder cannot
  // come from shank2.toml on the console. An explicit
  // path always wins.
  void OnConfigurePaths(rex::PathConfig& paths) override {
    std::error_code ec;
    const auto folder = rex::filesystem::GetExecutableFolder();
#if REX_PLATFORM_SWITCH
    // Installed as an NSP (nsp/): the folder the loader names in argv[0] holds the settings, saves and logs, and may
    // not exist yet on a fresh card.
    if (!folder.empty()) {
      std::filesystem::create_directories(folder, ec);
    }
#endif
    if (!paths.game_data_root.empty()) {
      return;
    }
#if REX_PLATFORM_SWITCH
    // The NSP carries the game data in its own RomFS (shank2_nsp.cpp); the SD card copy is only for the NRO.
    if (const std::string nsp_root = shank2::MountNspGameData(); !nsp_root.empty()) {
      paths.game_data_root = nsp_root;
      return;
    }
#endif
    if (folder.empty()) {
      return;
    }
    const auto candidate = folder / "game_root";
    if (std::filesystem::is_regular_file(candidate / "default.xex", ec) ||
        std::filesystem::is_regular_file(std::filesystem::path(candidate).concat(".nxpack"), ec)) {
      paths.game_data_root = candidate;
    }
  }

#if SHANK2_RENUT_RENDERER
  // With the Renut renderer there is no SDK graphics system and no GPU plugin: the renderer owns its window
  // swapchain and its render thread.
  void OnPreSetup(rex::RuntimeConfig& config) override {
    if (banjo::renut_gpu::Active()) {
      renut::gpu::ConfigureRuntime(config);
      REXLOG_INFO("graphics: Renut's Direct3D-level renderer (sa2_renderer = renut: no PM4 ring, no EDRAM)");
    }
  }

  // The renderer draws the SDK's menus through its own plume drawer, resizes its swapchain itself, and starts once
  // the window exists (here, before the game: the paths are final).
  std::unique_ptr<rex::ui::ImmediateDrawer> OnCreateImmediateDrawer() override {
    if (banjo::renut_gpu::Active()) return renut::gpu::CreateOverlayDrawer();
    return rex::ReXApp::OnCreateImmediateDrawer();
  }
  void OnWindowPixelSizeChanged(uint32_t pixel_width, uint32_t pixel_height) override {
    rex::ReXApp::OnWindowPixelSizeChanged(pixel_width, pixel_height);
    if (banjo::renut_gpu::Active()) renut::gpu::OnWindowResized();
  }
  std::optional<rex::PathConfig> OnFinalizePaths(const rex::PathConfig& defaults,
                                                 std::function<void(rex::PathConfig)> resume) override {
    if (banjo::renut_gpu::Active()) renut::gpu::StartRenderer(app_context(), [this] { return imgui_drawer(); });
    return rex::ReXApp::OnFinalizePaths(defaults, std::move(resume));
  }
  void OnPostSetup() override {
    if (banjo::renut_gpu::Active())
      sa2::capture::StartWithSource([](rex::ui::RawImage& image) { return renut::gpu::CaptureFrame(image); });
  }
  void OnShutdown() override {
    if (banjo::renut_gpu::Active()) renut::gpu::ShutdownRenderer();
    sa2::capture::Stop();
  }
#endif
};
