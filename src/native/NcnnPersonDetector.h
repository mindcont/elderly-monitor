#pragma once

#include "Detection.h"
#include <cstddef>
#include <memory>

class NcnnPersonDetector final : public PersonDetector {
public:
    NcnnPersonDetector(const std::string &paramPath, const std::string &binPath, int inputSize);
    ~NcnnPersonDetector() override;
    PersonDetection detectBgr(const unsigned char *pixels, int width, int height, std::size_t stride);
    PersonDetection detect(const FrameInfo &frame) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
