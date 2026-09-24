#include "NcnnPersonDetector.h"

#include <net.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>

struct NcnnPersonDetector::Impl {
    ncnn::Net net;
    bool ready = false;
    int inputSize = 320;
};

NcnnPersonDetector::NcnnPersonDetector(const std::string &paramPath, const std::string &binPath, int inputSize)
    : impl_(std::make_unique<Impl>())
{
    impl_->inputSize = inputSize;
    impl_->net.opt.num_threads = 2;
    impl_->net.opt.use_vulkan_compute = false;
    if (impl_->net.load_param(paramPath.c_str()) != 0 || impl_->net.load_model(binPath.c_str()) != 0)
        throw std::runtime_error("cannot load NCNN model: " + paramPath);
    impl_->ready = true;
}

NcnnPersonDetector::~NcnnPersonDetector() = default;

PersonDetection NcnnPersonDetector::detectBgr(const unsigned char *pixels, int width, int height, std::size_t stride)
{
    if (!impl_->ready || pixels == nullptr || width <= 0 || height <= 0)
        return {};

    ncnn::Mat input = ncnn::Mat::from_pixels_resize(
        pixels, ncnn::Mat::PIXEL_BGR, width, height, impl_->inputSize, impl_->inputSize);
    input.substract_mean_normalize(nullptr, std::vector<float>{1.0F / 255.0F}.data());

    auto extractor = impl_->net.create_extractor();
    extractor.set_light_mode(true);
    if (extractor.input("in0", input) != 0)
        return {};

    ncnn::Mat output;
    if (extractor.extract("out0", output) != 0)
        return {};

    PersonDetection result;
    if (output.dims != 2 || output.h < 5)
        return result;

    // Ultralytics NCNN export: [84, 8400], channels 0..3 are boxes and
    // channel 4 is the COCO person score for class 0.
    const float *personScores = output.row(4);
    for (int i = 0; i < output.w; ++i)
        result.confidence = std::max(result.confidence, personScores[i]);
    result.personPresent = result.confidence >= 0.35F;
    (void)stride; // appsink currently negotiates tightly packed BGR frames.
    return result;
}

PersonDetection NcnnPersonDetector::detect(const FrameInfo &)
{
    return {};
}
