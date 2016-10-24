// NumberOfThreadsExperiment.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <CommonCode/CommonCode.h>

#include <string>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>


void
singleThreadSort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, const std::string& callName)
{
  Timer timerThread;
  timerThread.Start();

  std::sort(begin, end, lessX);

  unsigned long dur = timerThread.GetMs();

  std::cout << "Report: " << callName << ", duration (ms): " << dur << '\n';
}

void
multiThreadSort(VectorOfPoint& data, size_t threadCount, size_t maxNumberOfThreads)
{
  size_t additionalThreadsCount = threadCount - 1;

  size_t dataSize = data.size();
  size_t partSize = dataSize / maxNumberOfThreads;

  VectorOfPoint::iterator partBegin = data.begin();
  VectorOfPoint::iterator partEnd   = partBegin + partSize;

  std::vector<std::thread> additionalThreads;

  const std::string callPrefix = "multiThreadSort_" + std::to_string(threadCount) + '_';
  std::string callName;

  Timer timerThread;
  timerThread.Start();
  for (size_t i = 0; i < additionalThreadsCount; i++)
  {
    callName = callPrefix;
    callName += std::to_string(i + 1);

    additionalThreads.push_back(std::thread(singleThreadSort, partBegin, partEnd, callName));

    partBegin = partEnd;
    partEnd   = partBegin + partSize;
  }

  callName = callPrefix;
  callName += "Main";

  singleThreadSort(partBegin, partEnd, callName);

  JoinThreads joiner(additionalThreads);
  joiner.Join();

  unsigned long dur = timerThread.GetMs();
  std::cout << "Threads time is (ms): " << dur << '\n';
}

void
ProcessFile(const std::string& fileName)
{
  std::cout << "Processing file: '" << fileName << '\'' << '\n';

  std::cout << "Reading data from file..." << '\n';

  std::ifstream inputStream(fileName);

  VectorOfPoint inputData;
  unsigned long readfileDuration = timeFuncInvocation(ReadInputTextData, inputStream, inputData);

  std::cout << "Read file duration is (ms): " << readfileDuration << '\n';

  std::cout << "  Number of points read: " << inputData.size() << '\n';

  std::cout << "Processing points..." << '\n';

  VectorOfPoint::iterator dataBegin;
  VectorOfPoint::iterator dataEnd;

  constexpr size_t maxNumberOfThreads = 10;
  unsigned long multiThreadDuration[maxNumberOfThreads + 1];

  VectorOfPoint inputDataSingleThread = inputData;
  dataBegin = inputDataSingleThread.begin();
  dataEnd   = inputDataSingleThread.begin() + inputDataSingleThread.size() / maxNumberOfThreads;

  std::cout << "------------------------------" << '\n';
  multiThreadDuration[1] = timeFuncInvocation(singleThreadSort, dataBegin, dataEnd, "singleThreadSort");

  VectorOfPoint inputDataMultiThread;

  for (size_t i = 2; i <= maxNumberOfThreads; i++)
  {
    inputDataMultiThread = inputData;
    std::cout << "------------------------------" << '\n';
    multiThreadDuration[i] = timeFuncInvocation(multiThreadSort, inputDataMultiThread, i, maxNumberOfThreads);
  }

  std::cout << '\n';
}

int
main(int argc, char* argv[])
{
  if (argc < 2)
    std::cout << "Please specify an input file name in the command-line." << '\n';

  std::cout << "----------============================================================----------\n";
  std::cout << "   This experiment demonstrates how threads affect each other." << '\n';
  std::cout << "   When the number of threads is not more than number of cores the interference" << '\n';
  std::cout << "   is minimal. When the number of threads is more than number of cores the" << '\n';
  std::cout << "   interference grows dramatically." << '\n';
  std::cout << "   Hyperthreading is not equal to the actual core. Hyperthreading does not" << '\n';
  std::cout << "   much help in this particular task (sorting of data: continuous" << '\n';
  std::cout << "   memory read/write)." << '\n';
  std::cout << "----------============================================================----------\n" << '\n' << '\n';

  for (int index = 1; index < argc; index++)
  {
    try
    {
      ProcessFile(argv[index]);
    }
    catch (const std::exception& e)
    {
      std::cerr << "Processing file error. File name: '" << argv[index] << "' Error message: " << e.what() << '\n';
    }
  }

  return 0;
}

