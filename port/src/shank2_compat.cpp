// Shank 2: stand-ins for the sa2-nx app services the Renut engine calls (src/renut_engine uses the sa2:: names as
// sa2-nx's app provides them). SA2 has real implementations (enhancement switches, resolution scale, capture); Shank 2
// has none of those yet, so each is a no-op or a safe default. Replace one when the feature is wanted.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#include <rex/platform.h>
#include <rex/ui/presenter.h>

#include "cpu/sa2_enhancements.h"
#include "me/sa2_render_mode.h"
#include "sa2_capture.h"

std::atomic<uint64_t> g_sa2_game_frames{0};

// Renut proves cached vertex / index / texture copies by SA2's generated "store cards"; Shank 2's code has none, so
// the renderer hashes them as it does without cards.
extern "C" const bool g_banjo_store_cards = false;

#if !REX_PLATFORM_SWITCH
// The console profiler's counter (the SDK defines it on the Switch).
extern "C" void RexSwitchPerfCount(unsigned) {}
#endif

// Frame pacing: a frame that misses its refresh goes at once (the player's default in sa2-nx).
SA2_ENHANCEMENT(g_vsync_relaxed, sa2_vsync_relaxed, sa2::enh::kNonCpu, true,
                "A frame that misses its refresh is shown as soon as it is ready");

namespace sa2::enh {

namespace {
std::vector<Enhancement*>& All() {
  static std::vector<Enhancement*> all;
  return all;
}
}  // namespace

bool Register(Enhancement* e) {
  All().push_back(e);
  return true;
}

void Resolve() {
  for (Enhancement* e : All()) e->active = e->own ? e->own() : false;
}

bool CpuMaster() { return true; }
bool NonCpuMaster() { return true; }
void StartReport() {}
void VerifyCheck(uint64_t) {}
void VerifyDifference() {}
bool VerifyMode() { return false; }
void CodegenChecksInit() {}
void CodegenChecksReport(int) {}
void FormsSelfCheck() {}
void ApplyGpuBoost() {}
bool GuestCodeMatches(const uint8_t*, uint32_t, uint32_t, uint64_t, const char*) { return true; }

}  // namespace sa2::enh

namespace sa2::render {

// Shank 2 draws at 1280x720 (the Xbox 360 game's own size); scale in quarters, 4 = 1x.
uint32_t WantedScaleQuarters() { return 4; }
uint32_t ScaleQuartersOf(bool) { return 4; }
bool Docked() { return false; }
int32_t RenderHeightOf(bool) { return 720; }
void ToggleDocked() {}
void OnPresented(uint64_t) {}
uint32_t MsaaSamples() { return 1; }
void NoteRescale() {}
void NoteMsaaUse(uint64_t) {}
void RegisterKeys() {}

}  // namespace sa2::render

namespace sa2::capture {

void Start(std::function<rex::ui::Presenter*()>) {}

// Debug (PC only): SHANK2_CAPTURE_EVERY_S=<seconds> saves the renderer's own frame as capture_<n>.ppm into
// SHANK2_CAPTURE_DIR (default: the current directory), up to SHANK2_CAPTURE_MAX files (default 12).
void StartWithSource(std::function<bool(rex::ui::RawImage&)> grab) {
#if !REX_PLATFORM_SWITCH
  const char* every = std::getenv("SHANK2_CAPTURE_EVERY_S");
  if (!every || !*every) return;
  const int period = std::max(1, std::atoi(every));
  const char* dir_env = std::getenv("SHANK2_CAPTURE_DIR");
  const std::string dir = dir_env && *dir_env ? dir_env : ".";
  const char* max_env = std::getenv("SHANK2_CAPTURE_MAX");
  const int max_files = max_env && *max_env ? std::max(1, std::atoi(max_env)) : 12;
  std::thread([grab = std::move(grab), period, dir, max_files] {
    for (int i = 0; i < max_files; ++i) {
      std::this_thread::sleep_for(std::chrono::seconds(period));
      rex::ui::RawImage image;
      if (!grab(image) || image.width == 0 || image.height == 0) continue;
      char name[512];
      std::snprintf(name, sizeof(name), "%s/capture_%02d.ppm", dir.c_str(), i);
      if (FILE* f = std::fopen(name, "wb")) {
        std::fprintf(f, "P6\n%u %u\n255\n", image.width, image.height);
        std::vector<uint8_t> row(size_t(image.width) * 3);
        for (uint32_t y = 0; y < image.height; ++y) {
          const uint8_t* src = image.data.data() + size_t(y) * image.stride;
          for (uint32_t x = 0; x < image.width; ++x) {
            row[x * 3 + 0] = src[x * 4 + 0];
            row[x * 3 + 1] = src[x * 4 + 1];
            row[x * 3 + 2] = src[x * 4 + 2];
          }
          std::fwrite(row.data(), 1, row.size(), f);
        }
        std::fclose(f);
      }
    }
  }).detach();
#endif
}
bool RenutLockstepOn() { return false; }
void RenutLockstepFrame(uint64_t, const rex::ui::RawImage*) {}
void Stop() {}
void BeforeFlip() {}

}  // namespace sa2::capture
