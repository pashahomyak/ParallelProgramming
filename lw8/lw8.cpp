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
#include "BlurBMPTask.h"
#include "ITask.h"

using namespace std;

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
    
    size_t inputFilesCount = inputFiles.size();

    //генерация очереди 
    for (int c = 0; c < inputFilesCount; c++)
    {
        SimpleBMP inputBmp;
        inputBmp.load(inputFiles[c].c_str());

        int bmpWidth = inputBmp.getWidth();
        int bmpHeigth = inputBmp.getHeight();

        div_t divider = div(bmpWidth, numberOfBlocksAtBreakage);

        vector<ITask*> tasks;

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

            ThreadData data = { &inputBmp, startIndex, endIndex };

            tasks.push_back(new BlurBMPTask(data));
        }
        
        int handleSize = numberOfBlocksAtBreakage;

        if (handlingMode == "1t1o")
        {
            HANDLE* handles = new HANDLE[handleSize];

            for (int j = 0; j < handleSize; j++)
            {
                handles[j] = CreateThread(NULL, 0, &ThreadProc, tasks[j], 0, NULL);
            }

            WaitForMultipleObjects(handleSize, handles, true, INFINITE);
        }
        else if (handlingMode == "pool")
        {
            ThreadPool threadPool(tasks, numberOfPoolStreams);
        }
        else
        {
            throw exception("Invalid handling mode.");
        }

        string outputPath = outputDirectoryPath + "/result" + to_string(c) + ".bmp";
        inputBmp.save(outputPath.c_str());
        inputBmp.destroy();
    }

    chrono::time_point<std::chrono::system_clock> endTime = chrono::system_clock::now();
    std::cout << "handlingMode: " << handlingMode << " time: " << chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << " ms";

    return 0;
}