#include "YoloDetector.h"
void YoloDetector::process(const Frame &frame) { emit detectionsReady({}, frame); }
