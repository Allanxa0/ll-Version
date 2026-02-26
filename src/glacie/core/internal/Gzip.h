#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <zlib.h>

namespace gzip {

inline bool decompress(const std::string& compressed_str, std::string& decompressed_str) {
    if (compressed_str.empty()) return false;

    z_stream d_stream{}; 
    
    if (inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK) { 
        return false; 
    }

    d_stream.next_in  = reinterpret_cast<Bytef*>(const_cast<char*>(compressed_str.data()));
    d_stream.avail_in = static_cast<uInt>(compressed_str.size());

    char outbuffer[4096]; 
    std::string outstring;

    int status;
    do {
        d_stream.next_out  = reinterpret_cast<Bytef*>(outbuffer);
        d_stream.avail_out = sizeof(outbuffer);

        status = inflate(&d_stream, Z_NO_FLUSH);

        if (status < 0) { 
            inflateEnd(&d_stream);
            return false;
        }

        uInt have = sizeof(outbuffer) - d_stream.avail_out;
        if (have > 0) {
            outstring.append(outbuffer, have);
        }
    } while (status != Z_STREAM_END && d_stream.avail_in != 0);

    inflateEnd(&d_stream);
    decompressed_str = std::move(outstring);
    return (status == Z_STREAM_END || status == Z_OK);
}

inline bool compress(const std::string& original_str, std::string& str) {
    if (original_str.empty()) return false;

    z_stream d_stream{};
    
    if (deflateInit2(&d_stream, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return false;
    }

    d_stream.next_in  = reinterpret_cast<Bytef*>(const_cast<char*>(original_str.data()));
    d_stream.avail_in = static_cast<uInt>(original_str.size());

    int status;
    char outbuffer[4096];
    std::string outstring;

    do {
        d_stream.next_out  = reinterpret_cast<Bytef*>(outbuffer);
        d_stream.avail_out = sizeof(outbuffer);

        status = deflate(&d_stream, Z_FINISH);

        uInt have = sizeof(outbuffer) - d_stream.avail_out;
        if (have > 0) {
            outstring.append(outbuffer, have);
        }
    } while (status == Z_OK);

    deflateEnd(&d_stream);

    if (status != Z_STREAM_END) {
        return false;
    }

    str = std::move(outstring);
    return true;
}

} 
