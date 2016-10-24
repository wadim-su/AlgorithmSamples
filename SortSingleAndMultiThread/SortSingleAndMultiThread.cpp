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
mergeContainerReady(VectorOfPoint::iterator begin, const VectorOfPoint::const_iterator middle,
                    const VectorOfPoint::const_iterator end, VectorOfPoint& mergedAux)
{
  if (mergedAux.size() < std::distance(VectorOfPoint::const_iterator(begin), end))
    throw std::logic_error("This function expects auxiliary container size enough to receive all data.");

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

      mergeContainerReady(begin, middle, end, mergedAux);

      //for (auto it = mergedAux.begin(); begin != end; ++begin, ++it)
      //  *begin = *it;
    }
    break;
  }
}

void
singleThreadMySort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, const std::string& callName)
{
  Timer timerThread;
  timerThread.Start();

  size_t distance = std::distance(begin, end);
  VectorOfPoint mergedAuxiliary(distance);

  mySort(begin, end, mergedAuxiliary);

  unsigned long dur = timerThread.GetMs();

  std::cout << "Report: " << callName << ", duration (ms): " << dur << '\n';
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
multiThreadSort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, const std::string& callName)
{
  size_t dataSize = std::distance(begin, end);

  size_t halfDataSize = dataSize / 2;
  VectorOfPoint::iterator middle = begin + halfDataSize;

  std::thread thePartner(singleThreadMySort, begin, middle, callName + "_Partner");
  singleThreadMySort(middle, end, callName + "_Main");

  thePartner.join();

  VectorOfPoint mergedAuxiliary(dataSize); // reserve needed size at initialization

  Timer timerMerge;
  timerMerge.Start();
  mergeContainerReady(begin, middle, end, mergedAuxiliary);
  //for (auto it = mergedAuxiliary.begin(); begin != end; ++begin, ++it)
  //  *begin = *it;

  unsigned long mergeDuration = timerMerge.GetMs();
  std::cout << "Merge duration is (ms): " << mergeDuration << '\n';
}

void
multi2ThreadSort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end)
{
  constexpr size_t coreCount = 2;
  constexpr size_t additionalThreadsCount = coreCount - 1;

  size_t dataSize = std::distance(begin, end);
  size_t partSize = dataSize / coreCount;

  VectorOfPoint::iterator partBegin = begin;
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

    additionalThreads.push_back(std::thread(singleThreadMySort, partBegin, partEnd, callName));

    partBegin = partEnd;
    partEnd = partBegin + partSize;
  }

  callName = callPrefix;
  callName += "Main";

  singleThreadMySort(partBegin, end, callName);

  JoinThreads joiner(additionalThreads);
  joiner.Join();

  unsigned long dur = timerThread.GetMs();
  std::cout << "Threads time is (ms): " << dur << '\n';

  VectorOfPoint mergedAuxiliary(dataSize); // reserve needed size at initialization

  Timer timerMerge;
  timerMerge.Start();
  mergeContainerReady(begin, partBegin, end, mergedAuxiliary);
  //for (auto it = mergedAuxiliary.begin(); begin != end; ++begin, ++it)
  //  *begin = *it;

  unsigned long mergeDuration = timerMerge.GetMs();
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

  VectorOfPoint inputData;
  unsigned long readfileDuration = timeFuncInvocation(ReadInputTextData, inputStream, inputData);

  std::cout << "Read file duration is (ms): " << readfileDuration << '\n';

  std::cout << "  Number of points read: " << inputData.size() << '\n';

  VectorOfPoint inputDataSingleThread = inputData;
  VectorOfPoint inputDataSingleMy     = inputData;
  VectorOfPoint inputDataMultiThread  = inputData;

  std::cout << "Processing points..." << '\n';

  size_t dataSizeBytes = sizeof(VectorOfPoint) * inputDataSingleThread.size();

  unsigned int coreCount = std::thread::hardware_concurrency();
  coreCount = 2;

  VectorOfPoint::iterator singleThreadSortBegin = inputDataSingleThread.begin();
  VectorOfPoint::iterator singleThreadSortEnd = inputDataSingleThread.end();

  unsigned long singleThreadDuration = timeFuncInvocation(singleThreadMySort, singleThreadSortBegin, singleThreadSortEnd, "singleThreadSort");
  unsigned long singleMyDuration     = timeFuncInvocation(singleThreadSort, inputDataSingleMy.begin(), inputDataSingleMy.end(), "singleThreadMySort");
  unsigned long multiThreadDuration  = timeFuncInvocation(multiThreadSort , inputDataMultiThread.begin(), inputDataMultiThread.end(), "multiThreadSort");

  if (inputDataSingleThread == inputDataMultiThread)
    std::cout << "The results of single- and milti-thread sorts do coincide." << '\n';
  else
    std::cout << "The results of single- and milti-thread sorts do not coincide!---------------------------" << '\n';

  if (inputDataSingleThread == inputDataSingleMy)
    std::cout << "The results of single- and My sorts do coincide." << '\n';
  else
    std::cout << "The results of single- and My sorts do not coincide!---------------------------" << '\n';

  std::cout << "Single thread sort duration is (ms): " << singleThreadDuration << '\n';
  std::cout << "Single thre Mysort duration is (ms): " << singleMyDuration << '\n';
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

