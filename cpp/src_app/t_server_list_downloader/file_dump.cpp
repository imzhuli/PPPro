#include "./file_dump.hpp"

#include <fstream>

bool DumpToFile(const std::filesystem::path & TargetFilename, const std::string_view & Data) {
    try {
        auto TempFilename = TargetFilename;
        TempFilename.concat(".tmp." + std::to_string(rand()));

        auto ofs = std::ofstream(TempFilename, std::ios::binary);
        if (!ofs) {
            return false;
        }
        ofs.write(Data.data(), Data.size());
        ofs.close();

        rename(TempFilename, TargetFilename);
    } catch (...) {
        return false;
    }
    return true;
}
