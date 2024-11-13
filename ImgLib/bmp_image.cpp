#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

    static const array<uint8_t, 2> BMP_SIG{ 'B', 'M' };

    static const uint32_t FILE_HEADER_SIZE = 14;
    static const uint32_t INFO_HEADER_SIZE = 40;

    PACKED_STRUCT_BEGIN BitmapFileHeader{
        // поля заголовка Bitmap File Header
        array<uint8_t, 2> signature = {'B', 'M'};
    // HEADER_SIZE + data size (padding * img_height)
        uint32_t sum_size = 0;
        uint32_t reserved_space = 0;
        uint32_t indent = FILE_HEADER_SIZE + INFO_HEADER_SIZE;
    }
    PACKED_STRUCT_END

    PACKED_STRUCT_BEGIN BitmapInfoHeader{
            uint32_t header_size = INFO_HEADER_SIZE;
            int32_t img_width = 0;
            int32_t img_height = 0;
            uint16_t plane_count = 1;
            uint16_t bit_by_pixel = 24;
            uint32_t compression_type = 0;
            // padding (size of one line of img in bytes) * img_height;
            uint32_t byte_in_data = 0;
            // approx. 300 DPI
            int32_t horizontal_resolution = 11811;
            int32_t vertical_resolution = 11811;
            int32_t used_colors_count = 0;
            int32_t significant_colors_count = 0x1000000;

    }
        PACKED_STRUCT_END

        // функция вычисления отступа по ширине
        static int GetBMPStride(int w) {
        return 4 * ((w * 3 + 3) / 4);
    }

    // напишите эту функцию
    bool SaveBMP(const Path& file, const Image& image) {
        ofstream out(file, ios::binary);
        if (!out) {
            return false;
        }

        const int w = image.GetWidth();
        const int h = image.GetHeight();
        const int stride = GetBMPStride(w);
        uint32_t data_size = stride * h;
        // set headers
        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;
        file_header.sum_size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + data_size;
        info_header.img_width = w;
        info_header.img_height = h;
        info_header.byte_in_data = data_size;

        // write headers
        out.write(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
        out.write(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

        vector<char> buff(stride);

        for (int y = h - 1; y >= 0; --y) {
            const Color* line = image.GetLine(y);
            for (int x = 0; x < w; ++x) {
                buff[x * 3] = static_cast<char>(line[x].b);
                buff[x * 3 + 1] = static_cast<char>(line[x].g);
                buff[x * 3 + 2] = static_cast<char>(line[x].r);
            }
            out.write(buff.data(), stride);
        }

        return out.good();
    }


    // напишите эту функцию
    Image LoadBMP(const Path& file) {
        ifstream ifs(file, ios::binary);
        if (!ifs) {
            return {};
        }
        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;

        // read headers
        ifs.read(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
        ifs.read(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

        // validate signature
        if (file_header.signature != BMP_SIG) {
            return {};
        }

        int w = info_header.img_width;
        int h = info_header.img_height;

        Image result(w, h, Color::Black());
        int stride = GetBMPStride(w);
        std::vector<char> buff(stride);

        for (int y = h - 1; y >= 0; --y) {
            Color* line = result.GetLine(y);
            ifs.read(buff.data(), stride);

            for (int x = 0; x < w; ++x) {
                line[x].r = static_cast<byte>(buff[x * 3 + 2]);
                line[x].g = static_cast<byte>(buff[x * 3 + 1]);
                line[x].b = static_cast<byte>(buff[x * 3]);
            }
        }

        return result;
    }

}  // namespace img_lib
