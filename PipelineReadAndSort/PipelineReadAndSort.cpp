// PipelineReadAndSort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <CommonCode/CommonCode.h>

#include <string>
#include <vector>
#include <queue>

#include <thread>
#include <mutex>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>


std::mutex readToSortMutex;
std::condition_variable readToSortCondition;
volatile bool readFinishedFlag;

std::mutex sortToMergeMutex;
std::condition_variable sortToMergeCondition;
volatile bool sortFinishedFlag;

struct PointRange
{
  VectorOfPoint::iterator begin;
  VectorOfPoint::iterator end;

  size_t beginIndex;
  size_t endIndex;

  PointRange() = default;

  PointRange(const VectorOfPoint::iterator& _begin, const VectorOfPoint::iterator& _end, size_t _beginIndex, size_t _endIndex) :
    begin(_begin),
    end  (_end),
    beginIndex(_beginIndex),
    endIndex  (_endIndex)
  {}
};

using VectorOfPointRange = std::vector<PointRange>;
using QueueOfPointRange = std::queue<PointRange>;

void
SortThreadProcedure(QueueOfPointRange& readToSortQueue, QueueOfPointRange& sortToMergeQueue, size_t inputDataSize)
{
  VectorOfPoint mergedAuxiliary(inputDataSize);

  while (true)
  {
    if (readFinishedFlag && readToSortQueue.empty())
    {
      sortFinishedFlag = true;
      break;
    }

    std::unique_lock<std::mutex> locker(readToSortMutex);

    readToSortCondition.wait(locker, [&] { return !readToSortQueue.empty(); });

    PointRange rangeToSort = readToSortQueue.front();
    readToSortQueue.pop();

    locker.unlock();

    mySort(rangeToSort.begin, rangeToSort.end, mergedAuxiliary);

    {
      std::lock_guard<std::mutex> locker2(sortToMergeMutex);
      sortToMergeQueue.push(rangeToSort);
      sortToMergeCondition.notify_one();
    }
  }
}

void
MergeThreadProcedure(QueueOfPointRange& sortToMergeQueue, size_t inputDataSize)
{
  class MergingStack
  {
  public:
    MergingStack(size_t inputDataSize) :
      _stack(),
      _mergedAuxiliary(inputDataSize)
    {}

    void PushAndMerge(const PointRange& orinaryRange)
    {
      _stack.push_back(orinaryRange);
      MergeTopRecursively();
    }

    void MergeAll()
    {
      for (int index = _stack.size() - 1; index >= 1; index--)
      {
        PointRange top = _stack[index];
        PointRange topButOne = _stack[index - 1];

        MergeTop(top, topButOne);
      }
    }

  private:
    VectorOfPointRange _stack;
    VectorOfPoint _mergedAuxiliary;

    void MergeTopRecursively()
    {
      size_t stackSize = _stack.size();

      if (stackSize == 0 || stackSize == 1)
        return; // nothing to merge

      size_t topIndex = stackSize - 1;
      size_t topButOneIndex = topIndex - 1;

      PointRange top = _stack[topIndex];
      PointRange topButOne = _stack[topButOneIndex];

      if (Distance(top) == Distance(topButOne))
      {
        MergeTop(top, topButOne);

        // Call 'merge' on the modified stack to check if further merge is possible.
        MergeTopRecursively();
      }
    }

    void MergeTop(PointRange top, PointRange topButOne)
    {
      // Merge points inside these 2 ranges.
      myMerge(topButOne.begin, top.begin, top.end, _mergedAuxiliary);

      // Pop these 2 ranges.
      _stack.pop_back();
      _stack.pop_back();

      // Push the new united range into the stack.
      PointRange unitedRange(topButOne.begin, top.end, topButOne.beginIndex, top.endIndex);
      _stack.push_back(unitedRange);
    }

    size_t Distance(const PointRange& range)
    {
      return (range.end - range.begin);
    }
  };

  MergingStack mergingStack(inputDataSize);

  while (true)
  {
    if (sortFinishedFlag && sortToMergeQueue.empty())
    {
      mergingStack.MergeAll();
      break;
    }

    std::unique_lock<std::mutex> locker(sortToMergeMutex);

    sortToMergeCondition.wait(locker, [&] { return !sortToMergeQueue.empty(); });

    PointRange rangeToMerge = sortToMergeQueue.front();
    sortToMergeQueue.pop();

    locker.unlock();

    mergingStack.PushAndMerge(rangeToMerge);
  }
}

void
ProcessFile(const std::string& fileName)
{
  std::cout << "Processing file: '" << fileName << '\'' << '\n';

  std::cout << "Reading data from file and sorting them simultaneously..." << '\n';

  Timer timer;
  timer.Start();

  std::ifstream inputStream(fileName);

  size_t numberOfPoints = 0;
  ReadNumberOfPoits(inputStream, numberOfPoints);

  VectorOfPoint inputData(numberOfPoints); // set needed size at construction
  VectorOfPoint inputDataForCheck;
  inputDataForCheck.reserve(numberOfPoints);

  size_t bundleSize = 5000; // This may be tuned
  //bundleSize = 50;
  //bundleSize = 499670;
  bundleSize = numberOfPoints / 100;
  size_t numberOfBundles = static_cast<size_t>(ceil((double)numberOfPoints / bundleSize));

  //std::mutex readToSortMutex;
  QueueOfPointRange readToSortQueue;
  //std::condition_variable readToSortCondition;

  //std::mutex sortToMergeMutex;
  QueueOfPointRange sortToMergeQueue;
  //std::condition_variable sortToMergeCondition;

  std::thread sorter(SortThreadProcedure, std::ref(readToSortQueue), std::ref(sortToMergeQueue), numberOfPoints);
  JoinThread joinSorter(sorter);

  std::thread merger(MergeThreadProcedure, std::ref(sortToMergeQueue), numberOfPoints);
  JoinThread joinMerger(merger);

  readFinishedFlag = false;
  sortFinishedFlag = false;

  size_t totalReadPointCount = 0;
  size_t readPointCount;
  PointRange readRange;
  readRange.begin = inputData.begin();
  readRange.beginIndex = 0;
  for (size_t bundle = 0; bundle < numberOfBundles; bundle++)
  {
    readPointCount = 0;
    ReadDataBundle(inputStream, bundleSize, readRange.begin, readPointCount);
    readRange.end = readRange.begin + readPointCount;
    readRange.endIndex = readRange.beginIndex + readPointCount;
    totalReadPointCount += readPointCount;

    for (auto it = readRange.begin; it != readRange.end; ++it)
      inputDataForCheck.push_back(*it);

    {
      std::lock_guard<std::mutex> locker(readToSortMutex);
      readToSortQueue.push(readRange);
      readToSortCondition.notify_one();
    }
    readRange.begin = readRange.end;
    readRange.beginIndex = readRange.endIndex;
  }

  unsigned long readFileDuration = timer.GetMs();

  readFinishedFlag = true;
  if (numberOfPoints == 0) // allow merge thread to finish
    sortFinishedFlag = true;

  if (totalReadPointCount != numberOfPoints)
    throw std::runtime_error("Number of points specified at the beginning of the stream does not coincide with actual number of points in the stream.");

  sorter.join();
  merger.join();

  unsigned long pipelineReadAndSortDuration = timer.GetMs();

  std::cout << "Check data size: " << inputDataForCheck.size() << '\n';

  std::cout << "Read file* duration is (ms): " << readFileDuration << '\n';
  std::cout << "  Number of points in the file: " << inputData.size() << '\n';

  std::sort(inputDataForCheck.begin(), inputDataForCheck.end(), lessX);

  if (inputData == inputDataForCheck)
    std::cout << "The results of my pipeline sort and std::sort do coincide." << '\n';
  else
    std::cout << "The results of my pipeline sort and std::sort do not coincide!---------------------------" << '\n';

  std::cout << "Pipeline read and sort duration is (ms): " << pipelineReadAndSortDuration << '\n';

  std::cout << "Sort to read data delay (ms): " << (pipelineReadAndSortDuration - readFileDuration) << '\n';

  std::cout << '\n';

  // К коду из книжки добавить флаг останова. Какой-нибудь volatile переменную? Атомик оператионз?
  // Создать два вспомогательных потока: для сортировки и для мерджа, запустить их на выполнение.
  // Вопомогательные потоки доложны остановиться сами, по флагу. А я потом сделаю им Join, чтобы правильно освободить
  // ресурсы в переменных потоков.
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
  return 0;
}

