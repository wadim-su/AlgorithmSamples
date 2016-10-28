// SortSingleAndMultiThread.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <CommonCode/CommonCode.h>

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <limits>
#include <iterator>

#include <chrono>
#include <thread>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>




void
sumData(VectorOfPoint::iterator begin, VectorOfPoint::iterator end)
{
  int sum = 0;
  for (; begin != end; ++begin)
  {
    sum += begin->x;
  }
}

void
singleThreadSortRecursive(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, VectorOfPoint& mergedAux, const std::string& callName)
{
  unsigned long duration = timeFuncInvocation(mySort, begin, end, mergedAux);

  std::cout << "Sort : " << callName << ", duration (ms): " << duration << '\n';
}

void
singleThreadStdSort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, const std::string& callName)
{
  Timer timerThread;
  timerThread.Start();

  std::sort(begin, end, lessX);
  //std::stable_sort(begin, end, lessX);
  //sumData(begin, end);

  unsigned long duration = timerThread.GetMs();

  std::cout << "Report: " << callName << ", duration (ms): " << duration << '\n';
}

void
multiThreadSortRecursive(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, VectorOfPoint& mergedAux,
  const std::string& callName, size_t currentLevel, size_t maxMultithreadLevel)
{
  size_t dataSize = std::distance(begin, end);
  size_t halfDataSize = dataSize / 2;
  VectorOfPoint::iterator middle = begin + halfDataSize;
  bool mergeNeeded = false;

  if (currentLevel < maxMultithreadLevel)
  {
    std::thread thePartner(multiThreadSortRecursive, begin, middle, mergedAux, callName + "_Partner", (currentLevel + 1), maxMultithreadLevel);
    multiThreadSortRecursive(middle, end, mergedAux, callName + "_Main", (currentLevel + 1), maxMultithreadLevel);
    thePartner.join();
    mergeNeeded = true;
  }
  else if (currentLevel == maxMultithreadLevel)
  {
    std::thread thePartner(singleThreadSortRecursive, begin, middle, mergedAux, callName + "_Partner");
    singleThreadSortRecursive(middle, end, mergedAux, callName + "_Main");
    thePartner.join();
    mergeNeeded = true;
  }
  else
    singleThreadSortRecursive(begin, end, mergedAux, callName + "_Main");

  if (mergeNeeded)
  {
    unsigned long mergeDuration = timeFuncInvocation(myMerge, begin, middle, end, mergedAux);
    std::cout << "Merge: " << callName << ", duration (ms): " << mergeDuration << '\n';
  }
}

void
singleThreadSort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, const std::string& callName)
{
  VectorOfPoint mergedAuxiliary(std::distance(begin, end));
  singleThreadSortRecursive(begin, end, mergedAuxiliary, callName);
}

void
multiThreadSort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, const std::string& callName, size_t maxMultithreadLevel)
{
  VectorOfPoint mergedAuxiliary(std::distance(begin, end));
  multiThreadSortRecursive(begin, end, mergedAuxiliary, callName, 1, maxMultithreadLevel);
}

void
ProcessFile(const std::string& fileName)
{
  std::cout << "Processing file: '" << fileName << '\''<< '\n';

  std::cout << "Reading data from file..." << '\n';

  Timer timerReadFile;
  timerReadFile.Start();

  std::ifstream inputStream(fileName);

  //VectorOfPoint inputData;
  //unsigned long readfileDuration = timeFuncInvocation(ReadInputTextData, inputStream, inputData);

  size_t numberOfPoints = 0;
  ReadNumberOfPoits(inputStream, numberOfPoints);

  VectorOfPoint inputData(numberOfPoints); // set needed size at construction

  size_t totalReadPointCount = 0;
  ReadDataBundle(inputStream, numberOfPoints, inputData.begin(), totalReadPointCount);

  unsigned long readFileDuration = timerReadFile.GetMs();

  std::cout << "Read file duration is (ms): " << readFileDuration << '\n';

  std::cout << "  Number of points read: " << inputData.size() << '\n';

  VectorOfPoint inputDataSingleThread = inputData;
  VectorOfPoint inputDataMultiThread  = inputData;

  std::cout << "Processing points..." << '\n';

  size_t dataSizeBytes = sizeof(VectorOfPoint) * inputDataSingleThread.size();

  unsigned int coreCount = std::thread::hardware_concurrency();
  size_t maxMultithreadLevel = coreCount / 2;

  unsigned long singleThreadDuration = timeFuncInvocation(singleThreadSort, inputDataSingleThread.begin(), inputDataSingleThread.end(), "singleThreadSort");
  unsigned long multiThreadDuration  = timeFuncInvocation(multiThreadSort , inputDataMultiThread.begin() , inputDataMultiThread.end() , "multiThreadSort", maxMultithreadLevel);

  std::sort(inputData.begin(), inputData.end(), lessX);

  if (inputData == inputDataSingleThread)
    std::cout << "The results of my single-thread sort and std::sort do coincide." << '\n';
  else
    std::cout << "The results of my single-thread sort and std::sort do not coincide!---------------------------" << '\n';

  if (inputData == inputDataMultiThread)
    std::cout << "The results of my multi-thread  sort and std::sort do coincide." << '\n';
  else
    std::cout << "The results of my multi-thread  sort and std::sort do not coincide!---------------------------" << '\n';

  std::cout << "Single thread sort duration is (ms): " << singleThreadDuration << '\n';
  std::cout << "Multi  thread sort duration is (ms): " << multiThreadDuration << '\n';

  if (singleThreadDuration > multiThreadDuration)
  {
    double persentage = singleThreadDuration == 0 ? 0 : (((double)singleThreadDuration - multiThreadDuration) / singleThreadDuration * 100);
    std::cout.precision(2);
    std::cout << "Multithreading gain is: " << std::fixed << persentage << " %" << '\n';
  }
  else
    std::cout << "Multithreading gain is: no gain." << '\n';

  std::cout << '\n';
}

int
main(int argc, char* argv[])
{
  if (argc < 2)
    std::cout << "Please specify an input file name in the command-line." << '\n';

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

  constexpr Point pt(10, 11);

  return 0;
}
