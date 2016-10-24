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
mergeContainerEmpty(VectorOfPoint::const_iterator itL, VectorOfPoint::const_iterator endL,
                    VectorOfPoint::const_iterator itR, VectorOfPoint::const_iterator endR,
                    VectorOfPoint& merged)
{
  if (merged.size() != 0)
    throw std::logic_error("This function expects output container is empty.");

  while ((itL != endL) && (itR != endR))
  {
    if (lessX(*itL, *itR))
    {
      merged.push_back(*itL);
      ++itL;
    }
    else
    {
      merged.push_back(*itR);
      ++itR;
    }
  }

  // It is possible that only one of (itL, itR) is not reached its end.
  for (; itL != endL; ++itL)
    merged.push_back(*itL);

  for (; itR != endR; ++itR)
    merged.push_back(*itR);
}

void
mergeContainerReady(VectorOfPoint::const_iterator itL, VectorOfPoint::const_iterator endL,
                    VectorOfPoint::const_iterator itR, VectorOfPoint::const_iterator endR,
                    VectorOfPoint& merged)
{
  if (merged.size() == 0)
    throw std::logic_error("This function expects output container exact size to receive all data.");

  auto mergedIt = merged.begin();

  while ((itL != endL) && (itR != endR))
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
  for (; itL != endL; ++itL, ++mergedIt)
    *mergedIt = *itL;

  for (; itR != endR; ++itR, ++mergedIt)
    *mergedIt = *itR;
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
singleThreadSort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, const std::string& callName)
{
  Timer timerThread;
  timerThread.Start();

  std::sort(begin, end, lessX);
  //std::stable_sort(begin, end, lessX);
  //sumData(begin, end);

  unsigned long dur = timerThread.GetMs();

  std::cout << "Report: " << callName << ", duration (ms): " << dur << '\n';
}

void
multi2ThreadSort(VectorOfPoint& data)
{
  constexpr size_t coreCount = 2;
  constexpr size_t additionalThreadsCount = coreCount - 1;

  size_t dataSize = data.size();
  size_t partSize = dataSize / coreCount;

  VectorOfPoint::iterator partBegin = data.begin();
  VectorOfPoint::iterator partEnd   = partBegin + partSize;

  std::vector<std::thread> additionalThreads;

  const std::string callPrefix = "multiThreadSort_";
  std::string callName;

  Timer timerThread;
  timerThread.Start();
  for (size_t i = 0; i < additionalThreadsCount; i++)
  {
    callName = callPrefix;
    callName += std::to_string(i + 1);

    additionalThreads.push_back(std::thread(singleThreadSort, partBegin, partEnd, callName));

    partBegin = partEnd;
    partEnd = partBegin + partSize;
  }

  callName = callPrefix;
  callName += "Main";

  singleThreadSort(partBegin, data.end(), "multiThreadSort_Main");

  JoinThreads joiner(additionalThreads);
  joiner.Join();

  unsigned long dur = timerThread.GetMs();
  std::cout << "Threads time is (ms): " << dur << '\n';

  VectorOfPoint unitedSorted(data.size()); // reserve needed size at initialization
  unsigned long mergeDuration = timeFuncInvocation(mergeContainerReady, data.begin(), partBegin, partBegin, data.end(), unitedSorted);
  data.swap(unitedSorted);
  std::cout << "Merge duration is (ms): " << mergeDuration << '\n';
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

  VectorOfPoint inputDataSingleThread;
  unsigned long readfileDuration = timeFuncInvocation(ReadInputTextData, inputStream, inputDataSingleThread);

  std::cout << "Read file duration is (ms): " << readfileDuration << '\n';

  std::cout << "  Number of points read: " << inputDataSingleThread.size() << '\n';

  VectorOfPoint inputDataMultiThread = inputDataSingleThread;
  VectorOfPoint inputDataTestThread = inputDataSingleThread;

  std::cout << "Processing points..." << '\n';

  size_t dataSizeBytes = sizeof(VectorOfPoint) * inputDataSingleThread.size();

  unsigned int coreCount = std::thread::hardware_concurrency();
  coreCount = 2;

  VectorOfPoint::iterator singleThreadSortBegin = inputDataSingleThread.begin();
  VectorOfPoint::iterator singleThreadSortEnd = inputDataSingleThread.end();

  unsigned long singleThreadDuration = timeFuncInvocation(singleThreadSort, singleThreadSortBegin, singleThreadSortEnd, "singleThreadSort");
  unsigned long multiThreadDuration  = timeFuncInvocation(multi2ThreadSort, inputDataMultiThread);

  if (inputDataSingleThread == inputDataMultiThread)
    std::cout << "The results of single- and milti-thread sorts do coincide." << '\n';
  else
    std::cout << "The results of single- and milti-thread sorts do not coincide!---------------------------" << '\n';

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

