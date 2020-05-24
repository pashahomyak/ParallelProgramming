#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include "simplebmp.h"

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

vector<RGBColor> GetVectorColor(int i, int j, SimpleBMP *bmp, int& pixelCounter)
{
    vector<RGBColor> resultVector;

    for (int k = -5; k <= 5; k++)
    {
        for (int l = -5; l <= 5; l++)
        {
            unsigned char red = 0, green = 0, blue = 0;

            if ((i+k) < bmp->getWidth() && (i+k) >= 0 && j+l < bmp->getHeight() && j+l >= 0)
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

    SimpleBMP* bmp = data -> bmp;

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
    if (argc != 5)
    {
        cout << "Invalig arguments." << endl << "blurbmp.exe inputBMP.bmp outputBMP.bmp numOfCores numOfThreads" << endl;
        return -1;
    }

    string inputFile(argv[1]);
    string outputFile(argv[2]);
    int numOfCores = atoi(argv[3]);
    int numOfThreads = atoi(argv[4]);

    //start timer
    chrono::time_point<std::chrono::system_clock> startTime = chrono::system_clock::now();
    
    SimpleBMP bmp;
    bmp.load(inputFile.c_str());

    int bmpWidth = bmp.getWidth();
    int bmpHeigth = bmp.getHeight();

    auto pixels = bmp.getPixels();

    div_t divider = div(bmpWidth, numOfThreads);

    ThreadData* threadDataParams = new ThreadData[numOfThreads];

    //деление по вертикали
    for (int i = 0; i < numOfThreads; i++)
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

        if (i == numOfThreads - 1)
        {
            endIndex = bmpWidth - 1;
        }
        else
        {
            endIndex = (i + 1) * divider.quot;
        }

        ThreadData data = {&bmp, startIndex, endIndex};

        threadDataParams[i] = data;
    }

    const DWORD_PTR mask = (1 << numOfCores) - 1;

    HANDLE* handles = new HANDLE[numOfThreads];
    for (int i = 0; i < numOfThreads; i++)
    {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadDataParams[i], CREATE_SUSPENDED, NULL);
        SetThreadAffinityMask(handles[i], mask);
    }

    // запуск потоков
    for (int j = 0; j < numOfThreads; j++)
    {
        ResumeThread(handles[j]);
    }

    // ожидание окончания работы потоков
    WaitForMultipleObjects(numOfThreads, handles, true, INFINITE);
    
    bmp.save(outputFile.c_str());

    //end Timer
    chrono::time_point<std::chrono::system_clock> endTime = chrono::system_clock::now();

    cout << "cores: " << numOfCores << " threads: " << numOfThreads << " time: "<< chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << " ms";

    delete[] threadDataParams;
    delete[] handles;
    bmp.~SimpleBMP();

    return 0;
}