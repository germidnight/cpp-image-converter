#include <bmp_image.h>
#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>

using namespace std;

namespace img_formats {
enum class Format {
    UNKNOWN,
    JPEG,
    PPM,
    BMP
};

class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path &file, const img_lib::Image &image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path &file) const = 0;
};

class ImageFormatPPM : public ImageFormatInterface{
public:
    bool SaveImage(const img_lib::Path &file, const img_lib::Image &image) const {
        return img_lib::SavePPM(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path &file) const {
        return img_lib::LoadPPM(file);
    }
};

class ImageFormatJPEG :public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path &file, const img_lib::Image &image) const {
        return img_lib::SaveJPEG(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path &file) const {
        return img_lib::LoadJPEG(file);
    }
};

class ImageFormatBMP : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path &file, const img_lib::Image &image) const {
        return img_lib::SaveBMP(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path &file) const {
        return img_lib::LoadBMP(file);
    }
};

Format GetFormatByExtension(const img_lib::Path &input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    if (ext == ".bmp"sv) {
        return Format::BMP;
    }

    return Format::UNKNOWN;
}
/*возвращает указатель на интерфейс нужного формата или nullptr, если формат не удалось определить.*/
ImageFormatInterface *GetFormatInterface(const img_lib::Path &path) {
    switch (GetFormatByExtension(path)) {
    case Format::JPEG: {
            ImageFormatJPEG* ptr = new ImageFormatJPEG();
            return ptr;
        }
    case Format::PPM: {
            ImageFormatPPM *ptr = new ImageFormatPPM();
            return ptr;
        }
    case Format::BMP: {
            ImageFormatBMP *ptr = new ImageFormatBMP();
            return ptr;
        }
    case Format::UNKNOWN: {
            return nullptr;
        }
    }
}
} // namespace img_formats

int main(int argc, const char **argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    img_formats::ImageFormatInterface* input_image_ptr = img_formats::GetFormatInterface(in_path);
    if (input_image_ptr == nullptr) {
        cout << "Unknown input image format"sv << endl;
        return 1;
    }
    img_formats::ImageFormatInterface *output_image_ptr = img_formats::GetFormatInterface(out_path);
    if (input_image_ptr == nullptr) {
        delete(input_image_ptr);
        cout << "Unknown output image format"sv << endl;
        return 2;
    }

    img_lib::Image image = input_image_ptr->LoadImage(in_path);
    if (!image) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!output_image_ptr->SaveImage(out_path, image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;

    delete(input_image_ptr);
    delete(output_image_ptr);
}