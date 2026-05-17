#include "caf-pack/TextureProcessor.hpp"
#include <fstream>
#include <cstring>

#ifdef HAVE_PNG
#include <png.h>
#endif

namespace CafPack {

bool TextureProcessor::canProcess(const std::filesystem::path& inputPath) const {
    return inputPath.extension() == ".png";
}

bool TextureProcessor::process(const std::filesystem::path& inputPath, CafData& outputData,
                               std::string& errorMessage) {
#ifndef HAVE_PNG
    errorMessage = "libpng not available - cannot process PNG files";
    return false;
#endif

    std::vector<uint8_t> pixels;
    uint32_t width, height;

    if (!loadPNG(inputPath, pixels, width, height)) {
        errorMessage = "Failed to load PNG: " + inputPath.string();
        return false;
    }

    if (!writeCafTexture(pixels, width, height, outputData)) {
        errorMessage = "Failed to write CAF texture";
        return false;
    }

    return true;
}

bool TextureProcessor::loadPNG(const std::filesystem::path& path, std::vector<uint8_t>& pixels,
                               uint32_t& width, uint32_t& height) {
#ifndef HAVE_PNG
    return false;
#else
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) return false;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(fp);
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        return false;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16) png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    pixels.resize(width * height * 4);

    std::vector<png_bytep> row_pointers(height);
    for (uint32_t y = 0; y < height; ++y) {
        row_pointers[y] = pixels.data() + y * width * 4;
    }

    png_read_image(png, row_pointers.data());

    png_destroy_read_struct(&png, &info, nullptr);
    fclose(fp);

    return true;
#endif
}

bool TextureProcessor::writeCafTexture(const std::vector<uint8_t>& pixels, uint32_t width,
                                       uint32_t height, CafData& output) {
    using namespace Caffeine::Assets;

    CafTextureMetadata metadata;
    metadata.header.magic = CAF_MAGIC;
    metadata.header.version = CAF_VERSION;
    metadata.header.assetType = static_cast<uint8_t>(CafAssetType::Texture);
    metadata.header.payloadSize = width * height * 4;
    metadata.header.flags = 0;
    metadata.width = width;
    metadata.height = height;
    metadata.format = static_cast<uint8_t>(TextureFormat::RGBA8);
    metadata.mipLevels = 1;

    output.resize(sizeof(CafTextureMetadata) + pixels.size());

    std::memcpy(output.data(), &metadata, sizeof(CafTextureMetadata));
    std::memcpy(output.data() + sizeof(CafTextureMetadata), pixels.data(), pixels.size());

    return true;
}

}  // namespace CafPack
