#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <fstream>
#include "simplebmp.h"
#include "LogBuffer.h"
#include "LogFileWriter.h"

using namespace std;

struct ThreadData
{
    SimpleBMP* bmp;
    LogBuffer* logBuffer;
    //ofstream* outputStream;
    int startIndex;
    int endIndex;
    int threadNumder;
    chrono::time_point<std::chrono::system_clock> startProgramTime;
};

struct RGBColor
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

enum ThreadPriority
{
    BELOW_NORMAL = -1,
    NORMAL,
    ABOVE_NORMAL
};

const int THREAD_PRIORITY_START_INDEX = 5;
const int BLUR_ITERATION_COUNT = 3;
const int BRUR_RADIUS = 5;

vector<RGBColor> GetVectorColor(int i, int j, SimpleBMP* bmp, int& pixelCounter)
{
    vector<RGBColor> resultVector;

    for (int k = -BRUR_RADIUS; k <= BRUR_RADIUS; k++)
    {
        for (int l = -BRUR_RADIUS; l <= BRUR_RADIUS; l++)
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

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    const ThreadData* data = static_cast<ThreadData*>(lpParam);

    SimpleBMP* bmp = data->bmp;
    //ofstream* outputStream = data->outputStream;

    for (int i = data->startIndex; i <= data->endIndex; i++)
    {
        for (int j = 0; j < bmp->getHeight(); j++)
        {
            //количество повторений
            for (int k = 0; k < BLUR_ITERATION_COUNT; k++)
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
            chrono::time_point<std::chrono::system_clock> endTime = chrono::system_clock::now();

            //*outputStream << data->threadNumder << " " << chrono::duration_cast<std::chrono::milliseconds>(endTime - data->startProgramTime).count() << endl;
            string resultTime = to_string(chrono::duration_cast<std::chrono::milliseconds>(endTime - data->startProgramTime).count());
            data->logBuffer->PushData(resultTime);
        }
    }

    ExitThread(0); // функция устанавливает код завершения потока в 0
}

int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        string firstArg(argv[1]);
        if (firstArg == "/?")
        {
            cout << "Usage Example" << endl << "lw4.exe inputBMP.bmp outputBMP.bmp numOfCores numOfThreads threadPrioritiesThroughDelimiterSpace" << endl;
            cout << "threadPrioritiesSpecification: " << "below-normal, normal, above-normal" << endl;
            return -1;
        }
    }

    if (argc < THREAD_PRIORITY_START_INDEX)
    {
        cout << "Invalid arguments. If you need help use: lw4.exe /?" << endl;
        return -1;
    }

    string inputFile(argv[1]);
    string outputFile(argv[2]);
    int numOfCores = atoi(argv[3]);
    int numOfThreads = atoi(argv[4]);

    if (argc != THREAD_PRIORITY_START_INDEX + numOfThreads)
    {
        cout << "Invalid arguments. If you need help use: lw4.exe /?" << endl;
        return -1;
    }

    //start timer
    chrono::time_point<std::chrono::system_clock> startTime = chrono::system_clock::now();

    vector<ThreadPriority> priorities;
    for (int i = THREAD_PRIORITY_START_INDEX; i < THREAD_PRIORITY_START_INDEX + numOfThreads; i++)
    {
        string priority(argv[i]);
        ThreadPriority threadPriority;

        if (priority == "below-normal")
        {
            threadPriority = ThreadPriority::BELOW_NORMAL;
        }
        else if (priority == "normal")
        {
            threadPriority = ThreadPriority::NORMAL;
        }
        else if (priority == "above-normal")
        {
            threadPriority = ThreadPriority::ABOVE_NORMAL;
        }

        priorities.push_back(threadPriority);
    }

    SimpleBMP bmp;
    bmp.load(inputFile.c_str());

    int bmpWidth = bmp.getWidth();
    int bmpHeigth = bmp.getHeight();

    auto pixels = bmp.getPixels();

    div_t divider = div(bmpWidth, numOfThreads);

    ThreadData* threadDataParams = new ThreadData[numOfThreads];
    ThreadPriority* threadPriorities = new ThreadPriority[numOfThreads];
    ofstream* outputStreams = new ofstream[numOfThreads];

    //logger
    LogFileWriter* logFileWriter = new LogFileWriter("outputLog.txt");
    LogBuffer* logBuffer = new LogBuffer(logFileWriter);

    //приоритеты и выходные потоки данных
    for (int i = 0; i < numOfThreads; i++)
    {
        threadPriorities[i] = priorities[i];
        outputStreams[i] = ofstream(to_string(i + 1) + "thread.txt");
    }

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

        ThreadData data = { &bmp, logBuffer, startIndex, endIndex, i + 1, startTime };

        threadDataParams[i] = data;
    }

    const DWORD_PTR mask = (1 << numOfCores) - 1;

    HANDLE* handles = new HANDLE[numOfThreads];
    for (int i = 0; i < numOfThreads; i++)
    {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadDataParams[i], CREATE_SUSPENDED, NULL);
        SetThreadAffinityMask(handles[i], mask);
        SetThreadPriority(handles[i], static_cast<int>(threadPriorities[i]));
    }

    // запуск потоков
    for (int j = 0; j < numOfThreads; j++)
    {
        ResumeThread(handles[j]);
    }

    // ожидание окончания работы потоков
    WaitForMultipleObjects(numOfThreads, handles, true, INFINITE);

    bmp.save(outputFile.c_str());

    delete[] threadDataParams;
    delete[] handles;
    bmp.~SimpleBMP();

    return 0;
}
