#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include "simplebmp.h"
#include "DirectoryReader.h"
#include "ThreadPool.h"

using namespace std;

struct ThreadData
{
    SimpleBMP* bmp;
    int startIndex;
    int endIndex;
};

struct RGBColor
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

vector<RGBColor> GetVectorColor(int i, int j, SimpleBMP* bmp, int& pixelCounter)
{
    vector<RGBColor> resultVector;

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

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    const ThreadData* data = static_cast<ThreadData*>(lpParam);

    SimpleBMP* bmp = data->bmp;

    for (int i = data->startIndex; i <= data->endIndex; i++)
    {
        for (int j = 0; j < bmp->getHeight(); j++)
        {
            //количество повторений
            for (int k = 0; k < 3; k++)
            {
                int sumR = 0; int sumG = 0; int sumB = 0; int pixelCounter = 0;

                vector<RGBColor> pixelVector = GetVectorColor(i, j, bmp, pixelCounter);

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

    ExitThread(0); // функция устанавливает код завершения потока в 0
}

int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        std::cout << "Invalig arguments." << endl 
            << "lw8.exe HandlingMode(pool/1t1o) NumberOfBlocksAtBreakage inputDirectoryPath outputDirectoryPath numberOfPoolStreams" << endl;
        return -1;
    }

    string handlingMode(argv[1]);
    int numberOfBlocksAtBreakage = atoi(argv[2]);
    string inputDirectoryPath(argv[3]);
    string outputDirectoryPath(argv[4]);
    int numberOfPoolStreams = atoi(argv[5]);

    //start timer
    chrono::time_point<std::chrono::system_clock> startTime = chrono::system_clock::now();

    if (!fs::exists(inputDirectoryPath))
    {
        throw exception("Invalid inputDirectoryPath.");
    }
    
    DirectoryReader directoryReader;
    std::vector<std::string> inputFiles = directoryReader.ReadDirectory(inputDirectoryPath);

    if (!fs::exists(outputDirectoryPath))
    {
        fs::create_directory(outputDirectoryPath);
    }
    
    for (int c = 0; c < inputFiles.size(); c++)
    {
        SimpleBMP bmp;
        bmp.load(inputFiles[c].c_str());

        int bmpWidth = bmp.getWidth();
        int bmpHeigth = bmp.getHeight();

        auto pixels = bmp.getPixels();

        div_t divider = div(bmpWidth, numberOfBlocksAtBreakage);

        ThreadData* threadDataParams = new ThreadData[numberOfBlocksAtBreakage];

        //деление по вертикали
        for (int i = 0; i < numberOfBlocksAtBreakage; i++)
        {
            int startIndex = 0;
            int endIndex = 0;

            if (i == 0)
            {
                startIndex = 0;
            }
            else
            {
                startIndex = (i * divider.quot) + 1;
            }

            if (i == numberOfBlocksAtBreakage - 1)
            {
                endIndex = bmpWidth - 1;
            }
            else
            {
                endIndex = (i + 1) * divider.quot;
            }

            ThreadData data = { &bmp, startIndex, endIndex };

            threadDataParams[i] = data;
        }

        int handleSize = numberOfBlocksAtBreakage;

        HANDLE* handles = new HANDLE[handleSize];
        if (handlingMode == "1t1o")
        {
            for (int j = 0; j < handleSize; j++)
            {
                handles[j] = CreateThread(NULL, 0, &ThreadProc, &threadDataParams[j], 0, NULL); //работает сразу после создания
            }

            WaitForMultipleObjects(handleSize, handles, true, INFINITE);
        }
        else if (handlingMode == "pool")
        {
            for (int j = 0; j < handleSize; j++)
            {
                handles[j] = CreateThread(NULL, 0, &ThreadProc, &threadDataParams[j], CREATE_SUSPENDED, NULL);
            }

            ThreadPool threadPool(handles, numberOfPoolStreams, handleSize);
            threadPool.Run();
        }
        else
        {
            throw exception("Invalid handling mode.");
        }

        string outputPath = outputDirectoryPath + "/result" + to_string(c) + ".bmp";
        bmp.save(outputPath.c_str());
    }

    chrono::time_point<std::chrono::system_clock> endTime = chrono::system_clock::now();
    std::cout << "handlingMode: " << handlingMode << " time: " << chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << " ms";

    return 0;
}