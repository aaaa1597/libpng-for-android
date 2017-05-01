//
// Created by jun on 2017/05/01.
//
#include <android/log.h>
#include "CppPng.h"

#define LOG_TAG ("PNG")
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

CppPng::CppPng(const std::string& file_name) :
        file_name_(file_name), data_(nullptr),
        width_(0), height_(0),
        bit_depth_(0), color_type_(0), interlace_type_(0),
        compression_type_(0), filter_method_(0)
{
    FILE* fp = fopen(file_name_.c_str(), "rb");
    if (fp == nullptr) {
        LOGE("%s is not found.", file_name_.c_str());
        fclose(fp);
        return;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr) {
        LOGE("png_create_read_struct error.");
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr) {
        LOGE("png_create_info_struct error.");
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        LOGE("png_jmpbuf error.");
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);

    unsigned int sig_bytes = 0;
    png_set_sig_bytes(png, sig_bytes);

    png_read_png(png, info, (PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND), nullptr);
    png_get_IHDR(png, info, &width_, &height_, &bit_depth_, &color_type_, &interlace_type_, &compression_type_, &filter_method_);

    unsigned int row_bytes = png_get_rowbytes(png, info);
    if (data_ != nullptr) {
        delete[] data_;
    }
    data_ = new unsigned char[row_bytes * height_];

    png_bytepp rows = png_get_rows(png, info);
    for (int i = 0; i < height_; ++i) {
        memcpy(data_ + (row_bytes * i), rows[i], row_bytes);
    }

    png_destroy_read_struct(&png, &info, nullptr);
}

CppPng::~CppPng()
{
    if (data_) delete[] data_;
}

unsigned int CppPng::get_width()
{
    return width_;
}

unsigned int CppPng::get_height()
{
    return height_;
}

unsigned char* CppPng::get_data()
{
    return data_;
}

bool CppPng::has_alpha()
{
    if (color_type_ == PNG_COLOR_TYPE_RGBA) {
        return true;
    }
    return false;
}