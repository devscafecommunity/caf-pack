#pragma once

/// Public alias header — caf-pack is the single source of truth for `.caf` / `.cap` types.
#include <caffeine/CafTypes.hpp>

namespace CafPack {

using CafHeader = Caffeine::Assets::CafHeader;
using CapHeader = Caffeine::Assets::CapHeader;
using CapEntry  = Caffeine::Assets::CapEntry;
using CafAssetType = Caffeine::Assets::CafAssetType;

}  // namespace CafPack
