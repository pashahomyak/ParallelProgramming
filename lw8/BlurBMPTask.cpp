#include "BlurBMPTask.h"

BlurBMPTask::BlurBMPTask(ThreadData& data) :
    threadData(data)
{
}

void BlurBMPTask::Execute()
{
    SimpleBMP* bmp = threadData.bmp;

    for (int i = threadData.startIndex; i <= threadData.endIndex; i++)
    {
        for (int j = 0; j < bmp->getHeight(); j++)
        {
            //количество повторений
            for (int k = 0; k < 3; k++)
            {
                int sumR = 0; int sumG = 0; int sumB = 0; int pixelCounter = 0;

                std::vector<RGBColor> pixelVector = GetVectorColor(i, j, bmp, pixelCounter);

                for (auto& item : pixelVector)
                {
                    sumR += item.red;
                    sumG += item.green;
                    sumB += item.blue;
                }

                bmp->setPixel(i, j, sumR / pixelCounter, sumG / pixelCounter, sumB / pixelCounter);
            }
        }
    }
}

std::vector<RGBColor> BlurBMPTask::GetVectorColor(int i, int j, SimpleBMP* bmp, int& pixelCounter)
{
    std::vector<RGBColor> resultVector;

    for (int k = -5; k <= 5; k++)
    {
        for (int l = -5; l <= 5; l++)
        {
            unsigned char red = 0, green = 0, blue = 0;

            if ((i + k) < bmp->getWidth() && (i + k) >= 0 && j + l < bmp->getHeight() && j + l >= 0)
            {
                bmp->getPixel(i + k, j + l, &red, &green, &blue);
                pixelCounter++;
                resultVector.push_back(RGBColor{ red, green, blue });
            }
        }
    }

    return resultVector;
}
