#include "GAI.h"
#include "GI.h"
#include <zlib.h>

namespace sfui { // move to utils or SRFormats?

class RamFileReader : public iFileReader {
public:
    RamFileReader(std::vector<std::uint8_t>&& data) : iFileReader(), m_data(data) {}

    void Read(void *data, std::size_t size, std::size_t offset) const final {
        if (offset + size > GetSize()) {
            throw std::ios_base::failure("The end of the given file stream has been reached", std::error_code(EINVAL, std::generic_category()));
        }

        std::memcpy(data, &m_data[offset], size);
    }

    std::size_t GetSize() const final {
        return m_data.size();
    }

private:
    const std::vector<std::uint8_t> m_data;
};

// TODO: it partly duplicates logic of `ZLibPKGItemFileReader`
static inline void unpackZL(std::vector<std::uint8_t>& data)
{
    constexpr std::uint32_t ZL02_SIGNATURE = 0x32304c5a;
    constexpr std::uint32_t ZL01_SIGNATURE = 0x31304c5a;

    if (data.size() < 8) {
        return;
    }

    std::uint32_t sig = ((const std::uint32_t *)data.data())[0];
    std::size_t outsize = ((const std::uint32_t *)data.data())[1];

    if (sig != ZL02_SIGNATURE && sig != ZL01_SIGNATURE) {
        return;
    }

    std::vector<std::uint8_t> outdata(outsize);

    int r = uncompress((Bytef *)outdata.data(), (uLongf *)&outsize,
            (const Bytef *)data.data() + 8, (uLong)(data.size() - 8));
    if (r != Z_OK) {
        throw std::runtime_error("Failed to decompress data: " + r);
    }

    outdata.resize(outsize);
    std::swap(outdata, data);
}

}

using namespace sfui;

LoaderPriorityLevel GAILoader::GetPriority(std::string_view extension) const noexcept
{
    if (extension == "gai" || extension == "GAI")
        return LoaderPriorityLevel::HighlyLikely;
    else
        return LoaderPriorityLevel::Unlikely;
}

bool GAILoader::Probe(const FileReader& reader) const
{
    if (reader.GetSize() < sizeof(uint32_t)) return false;
    constexpr std::uint32_t GAI_SIGNATURE = 0x00696167;
    auto signature = reader.Peek<std::uint32_t>();
    return signature == GAI_SIGNATURE;
}

ImageData GAILoader::Load(std::string_view filepath, FileReader&& reader) const
{
    reader.Skip(sizeof(std::uint32_t)); /* signature */
    if (auto version = reader.Read<std::uint32_t>(); version != 1) {
        throw std::runtime_error("GAI: invalid version");
    }

    auto startX = reader.Read<std::uint32_t>();
    auto startY = reader.Read<std::uint32_t>();
    auto finishX = reader.Read<std::uint32_t>();
    auto finishY = reader.Read<std::uint32_t>();
//std::cerr << __LINE__ << ", " << startX << ", " << startY << ", " << finishX << ", " << finishY << std::endl;

    auto frameCount = reader.Read<std::uint32_t>();
    if (frameCount < 1) {
        throw std::runtime_error("GAI: invalid frame count");
    }
//std::cerr << __LINE__ << ", " << frameCount << std::endl;

    auto hasBackground = reader.Read<std::uint32_t>(); // Animation has a background in separate file reader
//std::cerr << __LINE__ << ", hasBackground: " << hasBackground << std::endl;
    /*auto tSeek =*/ reader.Read<std::uint32_t>(); /* Wait seek? */
    /*auto tSize =*/ reader.Read<std::uint32_t>(); /* Wait size? */
    /*auto tUnknown1 =*/ reader.Read<std::uint32_t>(); /* Unknown */
    /*auto tUnknown2 =*/ reader.Read<std::uint32_t>(); /* Unknown */
//std::cerr << __LINE__ << ", " << tSeek << ", " << tSize << ", " << tUnknown1 << ", " << tUnknown2 << std::endl;

    struct GiFrame {
        std::uint32_t offset;
        std::uint32_t size;
    };
    static_assert(sizeof(GiFrame) == 8);

    auto data = std::make_shared<std::vector<sf::Texture>>();
    auto frames = reader.ReadVector<GiFrame>(frameCount);
    sfui::GiFrame background{startX, startY, finishX, finishY, rgba32_t{0x00000000}};
    if (hasBackground) {
        try {
            std::string backgroundPath(filepath);
            backgroundPath.erase(backgroundPath.size() - 2, 1); // Update extension: ".GAI" -> ".GI"
            FileReader giReader(backgroundPath);
            background = GILoader::LoadGiFrame(giReader);
        } catch (...) { // ignore exceptions here
std::cerr << "GAILoader: " << filepath << ": corresponded background GI file not found" << std::endl;
            //background = sfui::GiFrame{startX, startY, finishX, finishY, rgba32_t{0xff000000}}; // initialized with black color
        }
    }
    for (const auto& giFrame : frames) {
//std::cerr << __LINE__ << ", " << giFrame.offset << ", " << giFrame.size << std::endl;
        if (giFrame.size > 0) [[likely]] {
            reader.Seek(giFrame.offset);
            auto buf = reader.ReadVector<std::uint8_t>(giFrame.size);
            unpackZL(buf);
            FileReader giReader(std::make_unique<RamFileReader>(std::move(buf)));
//std::cerr << __LINE__ << ", " << giReader.GetSize() << std::endl;
            sfui::GiFrame frameImage = GILoader::LoadGiFrame(giReader, std::move(background));
            data->emplace_back(frameImage.ToTexture());
            background = hasBackground ? std::move(frameImage) : sfui::GiFrame{startX, startY, finishX, finishY, rgba32_t{0x00000000}};
        } else if (!data->empty()) [[unlikely]] {
            data->emplace_back(data->back());
        } else {
            data->emplace_back(background.ToTexture());
        }
    }
    return data;
}
