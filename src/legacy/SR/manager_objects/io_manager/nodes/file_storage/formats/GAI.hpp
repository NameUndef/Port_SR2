#ifndef SFUI_GAI_H_INCLUDED
#define SFUI_GAI_H_INCLUDED

#include "Loader.h"
#include "ImageData.h"
#include "../utils/FileReader.h"


namespace sfui {

class GAILoader : public iFileLoader<ImageData> {
public:
    ~GAILoader() final = default;
    LoaderPriorityLevel GetPriority(std::string_view extension) const noexcept final;
    bool Probe(const FileReader& file) const final;
    ImageData Load(std::string_view filepath, FileReader&& reader) const final;
    ImageData Load(std::string_view filepath) const { return Load(filepath, std::move(FileReader(filepath))); }
};

}

#endif // SFUI_GAI_H_INCLUDED
