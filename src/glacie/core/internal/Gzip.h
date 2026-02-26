#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Zlib.h"

namespace gzip {

inline bool decompress(const std::string& compressed_str, std::string& decompressed_str) {
    if (compressed_str.empty()) return false;
    
    auto result = Core::Compression::Zlib::decompress(compressed_str, false);
    if (result.has_value()) {
        decompressed_str = result.value();
        return true;
    }
    return false;
}

inline bool compress(const std::string& original_str, std::string& str) {
    if (original_str.empty()) return false;

    auto result = Core::Compression::Zlib::compress(original_str, 8, false);
    if (result.has_value()) {
        str = result.value();
        return true;
    }
    return false;
}

}
