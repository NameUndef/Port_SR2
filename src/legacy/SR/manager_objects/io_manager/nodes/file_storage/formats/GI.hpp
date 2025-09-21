#ifndef SFUI_GI_H_INCLUDED
#define SFUI_GI_H_INCLUDED

#include "Loader.h"
#include "ImageData.h"
#include "../utils/FileReader.h"


namespace sfui {

using rgba32_t = std::uint32_t;

/**
 * @brief A class representing a graphical image frame with RGBA pixel data.
 */
class GiFrame {
public:
    /**
     * @brief Constructs a GiFrame object with specified start and finish coordinates, and an optional fill color.
     *
     * @param startX The starting x-coordinate of the frame.
     * @param startY The starting y-coordinate of the frame.
     * @param finishX The finishing x-coordinate of the frame.
     * @param finishY The finishing y-coordinate of the frame.
     * @param fillColor The color to fill the frame with. Default is transparent (0x00000000).
     */
    GiFrame(std::size_t startX, std::size_t startY, std::size_t finishX, size_t finishY, rgba32_t fillColor = 0x00000000)
            : m_data((finishX - startX) * (finishY - startY), fillColor), m_startX(startX), m_startY(startY), m_finishX(finishX), m_finishY(finishY) {}

    /**
     * @brief Move constructor for GiFrame.
     *
     * @param frame The GiFrame object to move.
     */
    GiFrame(GiFrame&& frame) noexcept { *this = std::move(frame); }

    /**
     * @brief Move assignment operator for GiFrame.
     *
     * @param frame The GiFrame object to move.
     */
    GiFrame& operator=(GiFrame&& anotherFrame) noexcept {
        if (this != &anotherFrame) {
            m_data = std::move(anotherFrame.m_data);
            m_startX = anotherFrame.m_startX;
            m_startY = anotherFrame.m_startY;
            m_finishX = anotherFrame.m_finishX;
            m_finishY = anotherFrame.m_finishY;
        }
        return *this;
    }

    /**
     * @brief Accesses the pixel data at the specified coordinates.
     *
     * @param x The x-coordinate of the pixel.
     * @param y The y-coordinate of the pixel.
     * @return Reference to the pixel data at (x, y).
     */
    rgba32_t& At(std::size_t x, std::size_t y) { return m_data[(y - m_startY) * Width() + (x - m_startX)]; }

    /**
     * @brief Gets the width of the frame.
     *
     * @return The width of the frame.
     */
    std::size_t Width() const { return m_finishX - m_startX; }

    /**
     * @brief Gets the height of the frame.
     *
     * @return The height of the frame.
     */
    std::size_t Height() const { return m_finishY - m_startY; }

    /**
     * @brief Gets the x-coordinate of the frame.
     *
     * @return The x-coordinate.
     */
    std::size_t X() const { return m_startX; }

    /**
     * @brief Gets the y-coordinate of the frame.
     *
     * @return The y-coordinate.
     */
    std::size_t Y() const { return m_startY; }

    /**
     * @brief Provides access to the raw pixel data of the frame.
     *
     * @return Reference to the vector containing the pixel data.
     */
    std::vector<rgba32_t>& Pixels() { return m_data; }

    /**
     * @brief Converts the frame data to an SFML texture.
     *
     * @return An sf::Texture object containing the frame data.
     * @throws std::bad_alloc if the texture cannot be created.
     */
    sf::Texture ToTexture() const {
        sf::Texture texture;
        if (!texture.create(Width(), Height())) {
            throw std::bad_alloc();
        }
        texture.update(reinterpret_cast<const sf::Uint8*>(m_data.data()));
        return texture;
    }

private:
    GiFrame(const GiFrame&) = delete;
    GiFrame& operator=(const GiFrame&) = delete;

private:
    std::vector<rgba32_t> m_data;                           ///< The pixel data of the frame.
    std::size_t m_startX, m_startY, m_finishX, m_finishY;   ///< Rendering position of the pixel data.
};

class GILoader : public iFileLoader<ImageData> {
public:
    ~GILoader() final = default;

    LoaderPriorityLevel GetPriority(std::string_view extension) const noexcept final;
    bool Probe(const FileReader& file) const final;
    ImageData Load(std::string_view filepath, FileReader&& reader) const final;
    ImageData Load(std::string_view filepath) const { return Load(filepath, std::move(FileReader(filepath))); }

    static GiFrame LoadGiFrame(FileReader& file, GiFrame background = GiFrame{0,0,0,0,rgba32_t{0x00000000}});
};

}

#endif // SFUI_GI_H_INCLUDED
