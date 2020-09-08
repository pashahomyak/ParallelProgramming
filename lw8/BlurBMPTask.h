#pragma once
#include "ITask.h"
#include "simplebmp.h"
#include<vector>

struct ThreadData
{
    SimpleBMP* inputBmp;
    int startIndex;
    int endIndex;
};

struct RGBColor
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};
class BlurBMPTask: public ITask
{
public:
    BlurBMPTask(ThreadData& data);
    void Execute() override;
private:
    std::vector<RGBColor> GetVectorColor(int i, int j, SimpleBMP* bmp, int& pixelCounter);

    ThreadData threadData;
};