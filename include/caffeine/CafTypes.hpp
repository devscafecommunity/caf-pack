#pragma once
#include <cstdint>
#include <cstring>

namespace Caffeine {
namespace Assets {

// ══════════════════════════════════════════════════════════════════
// CAF Format (Caffeine Asset File)
// ══════════════════════════════════════════════════════════════════

// Magic identifier for .caf files
constexpr uint32_t CAF_MAGIC = 0x43414621;  // "CAF!"
constexpr uint32_t CAF_VERSION = 1;

// Asset type identifiers
enum class CafAssetType : uint8_t {
    Unknown = 0,
    Texture = 1,      // Image data (RGBA8, BC1, BC4, etc.)
    Audio = 2,        // PCM audio samples
    Mesh = 3,         // Vertex/index buffers
    Script = 4,       // Bytecode/text
    Animation = 5,    // Animation sequences
    Tileset = 6,      // Tilemap data
};

// Texture format identifiers
enum class TextureFormat : uint8_t {
    RGBA8 = 0,        // 32-bit RGBA
    BC1 = 1,          // DXT1 compression
    BC4 = 2,          // Single-channel compression
    BC5 = 3,          // Normal map compression
};

// Audio format identifiers
enum class AudioFormat : uint8_t {
    PCM16 = 0,        // 16-bit PCM
    PCM32 = 1,        // 32-bit float PCM
};

#pragma pack(push, 1)

// ── CAF Header (32 bytes, 32-byte aligned) ────────────────────────
struct CafHeader {
    uint32_t magic = CAF_MAGIC;      // 0x00: "CAF!"
    uint32_t version = CAF_VERSION;  // 0x04: Format version
    uint8_t  assetType = 0;          // 0x08: CafAssetType
    uint8_t  reserved[7] = {0};      // 0x09: Padding to 32 bytes
    uint32_t payloadSize = 0;        // 0x10: Total payload size (uncompressed)
    uint32_t flags = 0;              // 0x14: Bit flags (compressed, etc.)
    uint64_t crc64 = 0;              // 0x18: CRC64 checksum
};

static_assert(sizeof(CafHeader) == 32, "CafHeader must be 32 bytes");

// ── Texture Metadata (extends header) ─────────────────────────────
struct CafTextureMetadata {
    CafHeader header;
    uint16_t width = 0;              // 0x20: Image width in pixels
    uint16_t height = 0;             // 0x22: Image height in pixels
    uint8_t  format = 0;             // 0x24: TextureFormat
    uint8_t  mipLevels = 1;          // 0x25: Number of mipmap levels
    uint16_t reserved = 0;           // 0x26: Padding
    // Pixel data follows immediately at 32-byte alignment
};

// ── Audio Metadata (extends header) ──────────────────────────────
struct CafAudioMetadata {
    CafHeader header;
    uint32_t sampleRate = 44100;     // 0x20: Sample rate (Hz)
    uint32_t sampleCount = 0;        // 0x24: Total number of samples
    uint16_t channels = 2;           // 0x28: Channel count (1=mono, 2=stereo)
    uint8_t  format = 0;             // 0x2A: AudioFormat
    uint8_t  reserved = 0;           // 0x2B: Padding
    // PCM data follows immediately at 32-byte alignment
};

// ── Mesh Metadata (extends header) ────────────────────────────────
struct CafMeshMetadata {
    CafHeader header;
    uint32_t vertexCount = 0;        // 0x20: Number of vertices
    uint32_t indexCount = 0;         // 0x24: Number of indices
    uint16_t vertexStride = 0;       // 0x28: Bytes per vertex
    uint16_t indexFormat = 0;        // 0x2A: 0=uint16, 1=uint32
    // Vertex buffer follows at 32-byte alignment, then index buffer
};

#pragma pack(pop)

// ══════════════════════════════════════════════════════════════════
// CAP Format (Caffeine Asset Pack)
// ══════════════════════════════════════════════════════════════════

constexpr uint32_t CAP_MAGIC = 0x4341502F;  // "CAP/"
constexpr uint32_t CAP_VERSION = 1;

#pragma pack(push, 1)

// ── CAP Header (64 bytes) ──────────────────────────────────────────
struct CapHeader {
    uint32_t magic = CAP_MAGIC;      // 0x00: "CAP/"
    uint32_t version = CAP_VERSION;  // 0x04: Format version
    uint32_t assetCount = 0;         // 0x08: Number of assets in table
    uint32_t reserved1 = 0;          // 0x0C: Padding
    uint64_t tableOffset = 64;       // 0x10: Offset to CapEntry table
    uint64_t tableSize = 0;          // 0x18: Size of entry table
    uint64_t dataOffset = 0;         // 0x20: Offset to first .caf blob
    uint64_t totalSize = 0;          // 0x28: Total file size
    uint64_t crc64 = 0;              // 0x30: CRC64 of entire file
    uint32_t reserved2 = 0;          // 0x38: Reserved
    uint32_t reserved3 = 0;          // 0x3C: Reserved
};

static_assert(sizeof(CapHeader) == 64, "CapHeader must be 64 bytes");

// ── CAP Entry (hash-based lookup table) ────────────────────────────
struct CapEntry {
    uint64_t hashID = 0;             // MurmurHash3(path)
    uint64_t offset = 0;             // Absolute offset in .cap file
    uint32_t size = 0;               // Size of .caf blob
    uint32_t reserved = 0;           // Padding
};

static_assert(sizeof(CapEntry) == 24, "CapEntry must be 24 bytes");

#pragma pack(pop)

}  // namespace Assets
}  // namespace Caffeine
