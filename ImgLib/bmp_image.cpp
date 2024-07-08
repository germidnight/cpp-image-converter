#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <string_view>
#include <vector>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char signature[2] = {'B', 'M'};
    uint32_t total_length = 0;
    uint32_t reserved = 0;
    uint32_t file_begin_ident = 54;
}
PACKED_STRUCT_END

ostream& operator<<(ostream& output, BitmapFileHeader bfh) {
    const size_t buf_size = sizeof(BitmapFileHeader);
    typedef union {
        BitmapFileHeader bfh;
        char data[buf_size];
    } struct_converter;

    struct_converter buff = {bfh};
    output.write(buff.data, buf_size);
    return output;
}

istream& operator>>(istream& input, BitmapFileHeader& bfh) {
    input.read((char *)&bfh.signature[0], sizeof(bfh.signature[0]));
    input.read((char *)&bfh.signature[1], sizeof(bfh.signature[0]));
    input.read((char *)&bfh.total_length, sizeof(bfh.total_length));
    input.read((char *)&bfh.reserved, sizeof(bfh.reserved));
    input.read((char *)&bfh.file_begin_ident, sizeof(bfh.file_begin_ident));
    return input;
}

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t bih_length = 40;
    uint32_t image_width = 0;
    uint32_t image_height = 0;
    uint16_t plane_count = 1;
    uint16_t bits_per_pixel = 24;
    uint32_t compression_type = 0;
    uint32_t data_length = 0;
    int32_t  resolution_hor = 11811;    // 300 dpi
    int32_t  resolution_vert = 11811;   // 300 dpi
    int32_t  colors_used = 0;
    int32_t  colors = 0x1000000;    // 2^24
}
PACKED_STRUCT_END

ostream& operator<<(ostream& output, BitmapInfoHeader bih) {
    const size_t buf_size = sizeof(BitmapInfoHeader);
    typedef union {
        BitmapInfoHeader bfh;
        char data[buf_size];
    } struct_converter;

    struct_converter buff = {bih};
    output.write(buff.data, buf_size);
    return output;
}

istream& operator>>(istream& input, BitmapInfoHeader& bih) {
    input.read((char *)&bih.bih_length, sizeof(bih.bih_length));
    input.read((char *)&bih.image_width, sizeof(bih.image_width));
    input.read((char *)&bih.image_height, sizeof(bih.image_height));
    input.read((char *)&bih.plane_count, sizeof(bih.plane_count));
    input.read((char *)&bih.bits_per_pixel, sizeof(bih.bits_per_pixel));
    input.read((char *)&bih.compression_type, sizeof(bih.compression_type));
    input.read((char *)&bih.data_length, sizeof(bih.data_length));
    input.read((char *)&bih.resolution_hor, sizeof(bih.resolution_hor));
    input.read((char *)&bih.resolution_vert, sizeof(bih.resolution_vert));
    input.read((char *)&bih.colors_used, sizeof(bih.colors_used));
    input.read((char *)&bih.colors, sizeof(bih.colors));
    return input;
}

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    BitmapFileHeader bfh;
    BitmapInfoHeader bih;
    bih.image_width = static_cast<uint32_t>(image.GetWidth());
    const size_t width_stride = static_cast<size_t>(GetBMPStride(static_cast<int>(bih.image_width)));
    bih.image_height = static_cast<uint32_t>(image.GetHeight());
    bih.data_length = static_cast<uint32_t>(width_stride) * bih.image_height;
    bfh.total_length = bfh.file_begin_ident + bih.data_length;

    ofstream out(file, ios::binary);
    /* Пишем в файл заголовки */
    out << bfh << bih;
    /* Пишем саму картинку */
    vector<char> buff(width_stride, 0);

    for (int y = static_cast<int>(bih.image_height) - 1; y >= 0; --y) {
        const Color *line = image.GetLine(y);
        for (int x = 0; x < static_cast<int>(bih.image_width); ++x) {
            buff[static_cast<size_t>(x * 3 + 0)] = static_cast<char>(line[x].b);
            buff[static_cast<size_t>(x * 3 + 1)] = static_cast<char>(line[x].g);
            buff[static_cast<size_t>(x * 3 + 2)] = static_cast<char>(line[x].r);
        }
        /* в принципе, наверное лишнее */
        for (size_t x = bih.image_width * 3; x < width_stride; ++x) {
            buff[x] = 0;
        }
        out.write(buff.data(), static_cast<long long int>(width_stride));
    }
    return true;
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    BitmapFileHeader bfh;
    BitmapInfoHeader bih;
    ifstream ifs(file, ios::binary);

    ifs >> bfh >> bih;

    const size_t width_stride = static_cast<size_t>(GetBMPStride(static_cast<int>(bih.image_width)));

    Image result_image(static_cast<int>(bih.image_width),
                        static_cast<int>(bih.image_height), Color::Black());
    vector<char> buff(width_stride);

    for (int y = static_cast<int>(bih.image_height) - 1; y >= 0; --y) {
        Color *line = result_image.GetLine(y);
        ifs.read(buff.data(), static_cast<long long int>(width_stride));

        for (size_t x = 0; x < bih.image_width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }
    return result_image;
}

}  // namespace img_lib