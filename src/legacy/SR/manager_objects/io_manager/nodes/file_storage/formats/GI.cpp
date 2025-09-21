#include "GI.h"

#if defined(_WIN32) || defined(_WIN64)

uint32_t htobe32(uint32_t host_32bits)
{
    return ((host_32bits & 0x000000FF) << 24) |
           ((host_32bits & 0x0000FF00) << 8)  |
           ((host_32bits & 0x00FF0000) >> 8)  |
           ((host_32bits & 0xFF000000) >> 24);
}

#endif

#if 1 // Copied from OpenSR
//! Header of frame in *.gi file
struct GIFrameHeader
{
    uint32_t signature;  //!< Signature
    uint32_t version;  //!< Version of GI file
    uint32_t startX;  //!< Left corner
    uint32_t startY;  //!< Top corner
    uint32_t finishX;  //!< Right corner
    uint32_t finishY;  //!< Bottom corner
    uint32_t rBitmask;  //!< Mask of r color component
    uint32_t gBitmask;  //!< Mask of g color component
    uint32_t bBitmask;  //!< Mask of b color component
    uint32_t aBitmask;  //!< Mask of a color component
    uint32_t type;  //!< Frame type
    /*!<
    * Variants:
    *  -# 0 - One layer, 16 or 32 bit, depends on mask.
    *  -# 1 - One layer, 16 bit RGB optimized.
    *  -# 2 - Three layers:
    *   -# 16 bit RGB optimized - body
    *   -# 16 bit RGB optimized - outline
    *   -# 6 bit Alpha optimized
    *  -# 3 - Two layers:
    *   -# Indexed RGB colors
    *   -# Indexed Alpha
    *  -# 4 - One layer, indexed RGBA colors
    *  -# 5 - Delta frame of GAI animation.
    */
    uint32_t layerCount; //!< Number of layers in frame
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t unknown4;
};

//! Header of layer in *.gi files
struct GILayerHeader
{
    uint32_t seek;  //!< Layer offset in file
    uint32_t size;  //!< Layer size
    uint32_t startX;  //!< Layer left corner
    uint32_t startY;  //!< Layer top corner
    uint32_t finishX;  //!< Layer rigth corner
    uint32_t finishY;  //!< Layer bottom corner
    uint32_t unknown1;
    uint32_t unknown2;
};
#endif

/**
 * @class Pixel
 * @brief Simplifies access and modification of a 32-bit RGBA pixel or its individual color channels.
 *
 * This class provides an interface to manipulate individual color channels (Red, Green, Blue, Alpha) of a
 * 32-bit RGBA pixel, represented by a `rgba32_t` (32-bit unsigned integer).
 */
class Pixel {
public:
    enum /*class Channel*/ : uint8_t { Red, Green, Blue, Alpha };
    Pixel(rgba32_t& pixel) : m_pixel(pixel) {}
    Pixel& operator=(rgba32_t rgba32) { m_pixel = rgba32; return *this; }
    uint8_t& Channel(uint8_t channel) { return reinterpret_cast<uint8_t*>(&m_pixel)[channel]; }

private:
    rgba32_t& m_pixel;
};

/**
 * @class GiLayer
 * @brief Represents a layer of an image, allowing manipulation of a specific region of pixels.
 *
 * This class provides an interface to work with a specific rectangular region of pixels within an image
 * frame. It allows access to individual pixels in the specified layer.
 */
class GiLayer {
public:
    GiLayer(GiFrame& frame, size_t x, size_t y, size_t /*width*/, size_t /*height*/)
            : m_image(frame), m_x(x), m_y(y)/*, m_width(width), m_height(height)*/ {}
    Pixel at(size_t x, size_t y) { return Pixel(m_image.At(m_x + x, m_y + y)); }

private:
    GiFrame& m_image;
    size_t m_x, m_y;
    //size_t m_width, m_height; // unused
};

class BitsReader {
public:
    BitsReader(FileReader& reader) : m_reader(reader), m_lastByte(0), m_remainingBits(0) {}
    uint32_t ReadBits(uint8_t count) {
        constexpr uint8_t cBitsPerByte = 8;
        assert(count <= sizeof(uint32_t) * cBitsPerByte);

        // Check and read a new byte if no bits are left
            if (m_remainingBits == 0) {
            m_lastByte = m_reader.Read<uint8_t>();
            m_remainingBits = cBitsPerByte;
        }

        // Read full bytes if possible
            uint32_t result = 0;
        while (count > m_remainingBits) {
            result |= uint32_t(m_lastByte) << (count - m_remainingBits);
            count -= m_remainingBits;
            m_lastByte = m_reader.Read<uint8_t>();
            m_remainingBits = cBitsPerByte;
        }

        // Read the remaining bits from the current byte
            if (count > 0) {
            uint8_t mask = (1u << count) - 1u;
            result |= m_lastByte & mask;
            m_remainingBits -= count;
            m_lastByte >>= count;
        }

        return result;
    }

private:
    FileReader& m_reader;
    uint8_t m_lastByte;
    uint8_t m_remainingBits;
};

// Convert index of ARGB(little endian) to RGBA(big endian) used to select correct pixel color channel
static inline uint8_t channelIndexARGBToRGBA(uint8_t channelIndex)
{
    return ((3 - channelIndex) - 1) & 0x3;
}

/**
 * @brief Converts a color from R5G6B5 format to RGBA32 format.
 *
 * @param r5g6b5 The input color in R5G6B5 format (16 bits, little endian).
 * @return A 32-bit color in RGBA32 format, big endian.
 */
static inline rgba32_t R5G6B5ToRGBA32(uint16_t r5g6b5)
{
    uint32_t r = (((r5g6b5 >> 11) & 0x1f) << 3);
    uint32_t g = (((r5g6b5 >> 5) & 0x3f) << 2);
    uint32_t b = (((r5g6b5 >> 0) & 0x1f) << 3);
    uint32_t a = 0xff;
    return htobe32((r << 24) | (g << 16) | (b << 8) | (a << 0));
}

/**
 * @brief Converts a color from A6R5G6B5 format to RGBA32 format.
 *
 * @param a6r5g6b5 The input color in A6R5G6B5 format (32 bits, little endian).
 * @return A 32-bit color in RGBA32 format, big endian.
 */
static inline rgba32_t A6R5G6B5ToRGBA32(uint32_t a6r5g6b5)
{
    uint32_t a = ((63 - ((a6r5g6b5 >> 16) & 0x3f)) << 2);
    uint32_t r = (((a6r5g6b5 >> 11) & 0x1f) << 3);
    uint32_t g = (((a6r5g6b5 >> 5) & 0x3f) << 2);
    uint32_t b = (((a6r5g6b5 >> 0) & 0x1f) << 3);
    return htobe32((r << 24) | (g << 16) | (b << 8) | (a << 0));
}

/**
 * @brief Converts a color from ARGB32 format to RGBA32 format.
 *
 * @param argb32 The input color in ARGB32 format (32 bits, little endian).
 * @return A 32-bit color in RGBA32 format, big endian.
 */
static inline rgba32_t ARGB32ToRGBA32(uint32_t argb32)
{
    uint32_t a = ((argb32 >> 24) & 0xff);
    uint32_t r = ((argb32 >> 16) & 0xff);
    uint32_t g = ((argb32 >> 8) & 0xff);
    uint32_t b = ((argb32 >> 0) & 0xff);
    return htobe32((r << 24) | (g << 16) | (b << 8) | (a << 0));
}

#if 0 // For debug/reverse-engineering purposes
static void hexdump(FileReader& reader, size_t start, size_t size)
{
    constexpr size_t bytesPerLine = 16;

    auto oldOffset = reader.Tell();
    reader.Seek(start);
    for (size_t i = start / 16 * 16; i < start + size; i += bytesPerLine) {
        std::cerr << "0x" << std::setw(8) << std::setfill('0') << std::hex << i << ": ";
        for (size_t j = 0; j < bytesPerLine && i + j < start + size; ++j) {
            if (i + j < start) {
                std::cerr << "   ";
            } else {
                std::cerr << std::setw(2) << std::setfill('0') << std::hex << uint32_t(reader.Read<uint8_t>()) << ' ';
            }
        }
        std::cerr << std::endl;
    }
    std::cerr << std::dec; // Reset to decimal output
    reader.Seek(oldOffset);
}
#endif

static void loadR5G6B5(FileReader& reader, GiLayer pixels)
{
    reader.Skip(4); // pixels data size
    reader.Skip(12);

    uint32_t x = 0, y = 0;
    while (reader.Tell() < reader.GetSize()) {
        auto byte = reader.Read<uint8_t>();

        uint8_t pixelsCount = byte & 0x7f;
        bool pixelsFound = (byte >> 7) == 1;

        if (pixelsCount == 0) { // Goto new line
            x = 0;
            y++;
        } else if (pixelsFound) {
            for (uint8_t i = 0; i < pixelsCount; i++) {
                uint16_t color = reader.Read<uint16_t>();
                pixels.at(x++, y) = R5G6B5ToRGBA32(color);
            }
        } else { // Shift to right
            x += pixelsCount;
        }
    }
}

static void loadRGBI(FileReader& reader, GiLayer pixels)
{
    reader.Skip(4); // pixels data size
    reader.Skip(8);
    uint8_t paletteSize = reader.Read<uint8_t>();
    reader.Skip(3);

    // Load palette
    rgba32_t paletteRGBA32[paletteSize ?: 256];
    for (rgba32_t& color : paletteRGBA32) {
        color = R5G6B5ToRGBA32(reader.Read<uint16_t>());
    }

    uint32_t x = 0, y = 0;
    while (reader.Tell() < reader.GetSize()) {
        auto byte = reader.Read<uint8_t>();

        uint8_t pixelsCount = byte & 0x7f;
        bool pixelsFound = (byte >> 7) == 1;

        if (pixelsCount == 0) { // Goto new line
            x = 0;
            y++;
        } else if (pixelsFound) {
            for (uint8_t i = 0; i < pixelsCount; i++) {
                uint8_t index = reader.Read<uint8_t>();
                pixels.at(x++, y) = paletteRGBA32[index];
            }
        } else { // Shift to right
            x += pixelsCount;
        }
    }
}

static void loadAI(FileReader& reader, GiLayer pixels)
{
    reader.Skip(4); // pixels data size
    reader.Skip(8);
    uint8_t paletteSize = reader.Read<uint8_t>();
    reader.Skip(3);

    // Load palette
    rgba32_t paletteRGBA32[paletteSize ?: 256];
    for (auto& color : paletteRGBA32) {
        color = A6R5G6B5ToRGBA32(reader.Read<uint32_t>());
    }

    uint32_t x = 0, y = 0;
    while (reader.Tell() < reader.GetSize()) {
        auto byte = reader.Read<uint8_t>();

        uint8_t pixelsCount = byte & 0x7f;
        bool pixelsFound = (byte >> 7) == 1;

        if (pixelsCount == 0) { // Goto new line
            x = 0;
            y++;
        } else if (pixelsFound) {
            for (uint8_t i = 0; i < pixelsCount; i++) {
                uint8_t index = reader.Read<uint8_t>();
                pixels.at(x++, y) = paletteRGBA32[index];
            }
        } else {  // Shift to right
            x += pixelsCount;
        }
    }
}

static void loadA6(FileReader& reader, GiLayer pixels)
{
    reader.Skip(4); // pixels data size
    reader.Skip(12);

    uint32_t x = 0, y = 0;
    while (reader.Tell() < reader.GetSize()) {
        uint8_t byte = reader.Read<uint8_t>();

        uint8_t pixelsCount = byte & 0x7f;
        bool pixelsFound = byte >> 7 == 1;

        if (pixelsCount == 0) { // Goto new line
            x = 0;
            y++;
        } else if (pixelsFound) {
            for (uint8_t i = 0; i < pixelsCount; i++) {
                pixels.at(x++, y).Channel(Pixel::Alpha) = (63 - reader.Read<uint8_t>()) << 2;
            }
        } else { // Shift to right
            x += pixelsCount;
        }
    }
}

static void loadDeltaARGB(FileReader& reader, GiLayer pixels)
{
    reader.Skip(4); // pixels data size
    reader.Skip(4);
    uint32_t somethingCount = reader.Read<uint32_t>();
    uint16_t channelsShiftLeft = reader.Read<uint16_t>(); // 0x8888 for ARGB32(A8R8G8B8) and 0x5565 for A5R5G6B5
    reader.Skip(2);
    reader.Skip(somethingCount * sizeof(uint32_t));

#if 1 // DEBUG: negative channelShiftLeft value is undefined behavior!
    for (uint32_t i = 0; i < sizeof(rgba32_t); i++) {
        const uint8_t channelShiftLeft = 8 - (channelsShiftLeft >> (i * 4) & 0xf);
        assert(channelShiftLeft <= 8);
    }
#endif

    uint32_t channelIndex = 0;
    uint32_t x = 0, y = 0;
    for (auto byte = reader.Read<uint8_t>(); byte != 0x40; byte = reader.Read<uint8_t>()) {
        uint8_t pixelsData = byte & 0x7f;
        bool pixelsFound = byte >> 7 == 1;

        if (pixelsFound) {
            const uint8_t channelShiftLeft = 8 - (channelsShiftLeft >> (channelIndex * 4) & 0xf);
            const uint8_t rgbaChannel = channelIndexARGBToRGBA(channelIndex);
            uint8_t pixelsCount = (pixelsData & 0xf) + 1;
            int sign = ((pixelsData >> 6) & 1) ? -1 : +1;
            uint8_t bpp = 1 << ((pixelsData >> 4) & 3);
            BitsReader bitsReader(reader);

            while (pixelsCount > 0) {
                int channelDiffData = (bitsReader.ReadBits(bpp) + 1) << channelShiftLeft;
                pixels.at(x++, y).Channel(rgbaChannel) += sign * channelDiffData;
                pixelsCount--;
            }
        } else if (pixelsData == 0) { // Goto new channelIndex or line
            if (++channelIndex >= 4) {
                channelIndex = 0;
                y++;
            }
            x = 0;
        } else { // Shift to right
            if (pixelsData < 0x3f) {
                x += pixelsData;
            } else if (pixelsData == 0x3f) {
                auto pixelsCount = reader.Read<uint16_t>();
                x += pixelsCount;
            }
        }
    }
}

static void loadF6(FileReader& reader, GiLayer pixels)
{
    uint16_t channelsShiftLeft = reader.Read<uint16_t>(); // 0x8888 for ARGB32(A8R8G8B8) and 0x5565 for A5R5G6B5
    uint16_t blockCount = reader.Read<uint16_t>();

    BitsReader bitsReader(reader);
    for (int i = 0; i < blockCount; i++) {
        uint16_t blockX = bitsReader.ReadBits(10);
        uint16_t blockY = bitsReader.ReadBits(10);

        while (true) {
            uint8_t bpp = bitsReader.ReadBits(3);
            uint8_t channelIndex = bitsReader.ReadBits(2);

            if (bpp == 0 && channelIndex == 0) { // Goto new line
                blockY++;
            } else if (bpp == 0 && channelIndex == 1) { // end block
                break;
            } else {
                const uint8_t channelShiftLeft = 8 - (channelsShiftLeft >> (channelIndex * 4) & 0xf);
                const uint8_t rgbaChannel = channelIndexARGBToRGBA(channelIndex);
                int sign = (bpp && bitsReader.ReadBits(1)) ? -1 : +1;
                uint16_t x = blockX;
                uint16_t y = blockY;

                while (true) {
                    if (/*auto pixelsFound = */bitsReader.ReadBits(1)) {
                        if (bpp > 0) {
                            int channelDiffData = (bitsReader.ReadBits(bpp) + 1) << channelShiftLeft;
                            pixels.at(x++, y).Channel(rgbaChannel) += sign * channelDiffData;
                        } else { // fill zero
                            pixels.at(x++, y) = 0x00000000;
                        }
                    } else { // Shift to right
                        auto pixelsCount = bitsReader.ReadBits(3);
                        if (pixelsCount == 0) break;
                        x += pixelsCount;
                    }
                }
            }
        }
    }
}

static GiFrame loadGiFrame(uint32_t frameType, const std::vector<GILayerHeader>& layers, FileReader& reader, GiFrame base)
{
    const auto checkLayersCount = [frameType, &layers](std::size_t layersCount, std::size_t expectedLayersCount) {
        assert(layersCount == layers.size());
        if (layersCount < expectedLayersCount) {
            throw std::runtime_error(std::string("Invalid GI frame layers count: ") + std::to_string(layersCount) + ", expected layers count is " + std::to_string(expectedLayersCount) + " for Gi frame type " + std::to_string(frameType));
        }
        if (layersCount > expectedLayersCount) {
            std::cerr << "Warning! GiFrame type " << frameType << " has unexpected layers count: " << layersCount << ". Expected amount is " << expectedLayersCount << "." << std::endl;
        }
        return true;
    };
    const auto loadLayers = [&base, &layers, &reader, &checkLayersCount](std::initializer_list<void(*)(FileReader& reader, GiLayer pixels)> layersLoaders) {
        if (checkLayersCount(layers.size(), layersLoaders.size())) {
            auto layerLoaderIt = layersLoaders.begin();
            for (size_t layerIndex = 0; layerIndex < layersLoaders.size(); ++layerIndex, ++layerLoaderIt) {
                const auto& layerHeader = layers[layerIndex];
                const auto& layerLoader = *layerLoaderIt;

                if (layerHeader.size > 0) {
                    FileReader layerReader(std::make_unique<PartFileReader>(reader, layerHeader.seek, layerHeader.size));
                    (*layerLoader)(layerReader, {base, layerHeader.startX, layerHeader.startY, layerHeader.finishX - layerHeader.startX, layerHeader.finishY - layerHeader.startY});
                }
            }
        }
    };

//std::cerr << "gi frame type: " << frameType << "," << layers.size() << "," << layers[0].size << std::endl;
//std::cerr << "gi frame size: " << base.Width() << "," << base.Height() << std::endl;
    switch (frameType)
    {
    case 0:
        if (checkLayersCount(layers.size(), 1) && layers[0].size > 0) {
            FileReader layerReader(std::make_unique<PartFileReader>(reader, layers[0].seek, layers[0].size));
            if (layerReader.GetSize() >= base.Width() * base.Height() * sizeof(rgba32_t)) { // 32-bit argb image
                for (auto& pixel : base.Pixels()) {
                    pixel = ARGB32ToRGBA32(layerReader.Read<std::uint32_t>());
                }
            } else { // 16-bit r5g6b5 image
                for (auto& pixel : base.Pixels()) {
                    pixel = R5G6B5ToRGBA32(layerReader.Read<std::uint16_t>());
                }
            }
        }
        return base;
    case 1:
        loadLayers({loadR5G6B5});
        return base;
    case 2:
        loadLayers({loadR5G6B5, loadR5G6B5, loadA6});
        return base;
    case 3:
        loadLayers({loadRGBI, loadAI});
        return base;
    case 4:
        if (checkLayersCount(layers.size(), 2) && layers[0].size > 0 && layers[1].size > 0) {
            FileReader paletteReader(std::make_unique<PartFileReader>(reader, layers[1].seek, layers[1].size));
            auto paletteRGBA32 = paletteReader.ReadVector<rgba32_t>(layers[1].size / sizeof(rgba32_t));
            FileReader layerReader(std::make_unique<PartFileReader>(reader, layers[0].seek, layers[0].size));
            for (auto& pixel : base.Pixels()) {
                pixel = paletteRGBA32[layerReader.Read<std::uint8_t>()];
            }
        }
        return base;
    case 5:
        loadLayers({loadDeltaARGB});
        return base;
    case 6:
        loadLayers({loadF6});
        return base;
    default:
        throw std::runtime_error("Unknown GI frame type");
    }
}

using namespace sfui;

GiFrame GILoader::LoadGiFrame(FileReader& reader, GiFrame background)
{
    auto frameHeader = reader.Read<GIFrameHeader>();
    auto layersHeaders = reader.ReadVector<GILayerHeader>(frameHeader.layerCount);

    if (!background.Width() && !background.Height()) {
        background = GiFrame(frameHeader.startX, frameHeader.startY, frameHeader.finishX, frameHeader.finishY, rgba32_t{0x00000000});
    }
    return loadGiFrame(frameHeader.type, layersHeaders, reader, std::move(background));
}

LoaderPriorityLevel GILoader::GetPriority(std::string_view extension) const noexcept
{
    if (extension == "gi" || extension == "GI")
        return LoaderPriorityLevel::HighlyLikely;
    else
        return LoaderPriorityLevel::Unlikely;
}

bool GILoader::Probe(const FileReader& reader) const
{
    if (reader.GetSize() < sizeof(uint32_t)) return false;
    constexpr std::uint32_t GI_FRAME_SIGNATURE = 0x00006967;
    auto signature = reader.Peek<std::uint32_t>();
    return signature == GI_FRAME_SIGNATURE;
}

ImageData GILoader::Load(std::string_view /*filepath*//*TODO: use for printing errors*/, FileReader&& reader) const
{
    auto data = std::make_shared<std::vector<sf::Texture>>();
    data->emplace_back(LoadGiFrame(reader).ToTexture());
    return data;
}
