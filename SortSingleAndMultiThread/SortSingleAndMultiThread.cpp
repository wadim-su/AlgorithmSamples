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
myMerge(VectorOfPoint::iterator begin, const VectorOfPoint::const_iterator middle,
        const VectorOfPoint::const_iterator end, VectorOfPoint& mergedAux)
{
  // Turn it off for the sake of throughput.
  //if (mergedAux.size() < std::distance(VectorOfPoint::const_iterator(begin), end))
  //  throw std::logic_error("This function expects auxiliary container size enough to receive all data.");

  auto itL = begin;
  auto itR = middle;

  auto mergedIt = mergedAux.begin();

  while ((itL != middle) && (itR != end))
  {
    if (lessX(*itL, *itR))
    {
      *mergedIt = *itL;
      ++itL;
    }
    else
    {
      *mergedIt = *itR;
      ++itR;
    }
    ++mergedIt;
  }

  // It is possible that only one of (itL, itR) is not reached its end.
  for (; itL != middle; ++itL, ++mergedIt)
    *mergedIt = *itL;

  for (; itR != end; ++itR, ++mergedIt)
    *mergedIt = *itR;

  // Copy data back into the input storage.
  for (mergedIt = mergedAux.begin(); begin != end; ++begin, ++mergedIt)
    *begin = *mergedIt;
}

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
mySort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, VectorOfPoint& mergedAux)
{
  size_t dataSize = std::distance(begin, end);

  switch (dataSize)
  {
  case 1:
    // nothing to sort
    break;
  case 2:
    {
      VectorOfPoint::iterator second = begin + 1;
      if (!lessX(*begin, *second))
      {
        mergedAux[0] = *begin;
        *begin = *second;
        *second = mergedAux[0];
      }
    }
    break;
  default:
    {
      size_t halfDataSize = dataSize / 2;
      VectorOfPoint::iterator middle = begin + halfDataSize;
      mySort(begin, middle, mergedAux);
      mySort(middle, end, mergedAux);

      myMerge(begin, middle, end, mergedAux);
    }
    break;
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

bool
operator ==(const VectorOfPoint& left, const VectorOfPoint& right)
{
  if (left.size() != right.size())
    return false;

  for (auto it = left.begin(), end = left.end(), rightIt = right.begin(); it != end; ++it, ++rightIt)
  {
    if (it->x != rightIt->x) // sorting and comparison goes only by x-coordinate
      return false;
  }

  return true;
}

void
ProcessFile(const std::string& fileName)
{
  std::cout << "Processing file: '" << fileName << '\''<< '\n';

  std::cout << "Reading data from file..." << '\n';

  std::ifstream inputStream(fileName);

  VectorOfPoint inputData;
  unsigned long readfileDuration = timeFuncInvocation(ReadInputTextData, inputStream, inputData);

  std::cout << "Read file duration is (ms): " << readfileDuration << '\n';

  std::cout << "  Number of points read: " << inputData.size() << '\n';

  VectorOfPoint inputDataSingleThread = inputData;
  VectorOfPoint inputDataMultiThread  = inputData;

  std::cout << "Processing points..." << '\n';

  size_t dataSizeBytes = sizeof(VectorOfPoint) * inputDataSingleThread.size();

  unsigned int coreCount = std::thread::hardware_concurrency();
  size_t maxMultithreadLevel = coreCount / 2;
  //maxMultithreadLevel = 0;

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

