#include "stdafx.h"
#include "CommonCode.h"
#include <thread>
#include <istream>
#include <string>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct Point_T
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReadInputTextData(std::istream& inputStream, /*out*/VectorOfPoint& inputData)
{
  size_t pointCount = 0;
  if (!(inputStream >> pointCount))
    throw std::runtime_error("Error reading data from the given stream.");

  inputData.clear();
  inputData.reserve(pointCount);

  std::string strCurrentPoint;
  std::getline(inputStream, strCurrentPoint); // eat the rest of the first line

  size_t readPointCount = 0;
  Point currentPoint;
  while (std::getline(inputStream, strCurrentPoint))
  {
    std::istringstream strStream(strCurrentPoint);
    strStream >> currentPoint.x >> currentPoint.y;
    inputData.push_back(currentPoint);
    readPointCount++;
  }

  if (readPointCount != pointCount)
    throw std::runtime_error("Number of points specified at the beginning of the stream does not coincide with actual number of points in the stream.");
}

void
ReadNumberOfPoits(std::istream& inputStream, /*out*/size_t& pointCount)
{
  pointCount = 0;
  if (!(inputStream >> pointCount))
    throw std::runtime_error("Error reading data from the given stream.");

  std::string strCurrentPoint;
  std::getline(inputStream, strCurrentPoint); // eat the rest of the first line
}

void
ReadDataBundle(std::istream& inputStream, size_t bundleSize, VectorOfPoint::iterator current, /*out*/size_t& readPointCount)
{
  readPointCount = 0;
  std::string strCurrentPoint;
  Point currentPoint;
  while ((readPointCount < bundleSize) && std::getline(inputStream, strCurrentPoint))
  {
    std::istringstream strStream(strCurrentPoint);
    strStream >> currentPoint.x >> currentPoint.y;
    *current = currentPoint;
    ++current;
    readPointCount++;
  }
}

bool
operator ==(const VectorOfPoint& left, const VectorOfPoint& right)
{
  if (left.size() != right.size())
    return false;
  size_t index = 0;
  for (auto it = left.begin(), end = left.end(), rightIt = right.begin(); it != end; ++it, ++rightIt, ++index)
  {
    if (it->x != rightIt->x) // sorting and comparison goes only by x-coordinate
      return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Divide-and-conquer sorting functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
myMerge(VectorOfPoint::iterator begin, const VectorOfPoint::const_iterator& middle, const VectorOfPoint::const_iterator& end,
        VectorOfPoint::iterator mergeBegin, const VectorOfPoint::const_iterator& mergeEnd)
{
  // Turn it off for the sake of throughput.
  //if (std::distance(static_cast<VectorOfPoint::const_iterator>(mergeBegin), mergeEnd) < std::distance(VectorOfPoint::const_iterator(begin), end))
  //  throw std::logic_error("This function expects auxiliary container size enough to receive all data.");

  auto itL = begin;
  auto itR = middle;

  auto mergedIt = mergeBegin;

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
  for (; begin != end; ++begin, ++mergeBegin)
    *begin = *mergeBegin;
}

void
mySort(VectorOfPoint::iterator begin, const VectorOfPoint::const_iterator& end,
       VectorOfPoint::iterator mergeBegin, const VectorOfPoint::const_iterator& mergeEnd)
{
  size_t dataSize = std::distance(static_cast<VectorOfPoint::const_iterator>(begin), end);

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
      *mergeBegin = *begin;
      *begin = *second;
      *second = *mergeBegin;
    }
  }
  break;
  default:
  {
    size_t halfDataSize = dataSize / 2;
    VectorOfPoint::iterator middle      = begin      + halfDataSize;
    VectorOfPoint::iterator mergeMiddle = mergeBegin + halfDataSize;
    mySort(begin, middle, mergeBegin, mergeMiddle);
    mySort(middle, end, mergeMiddle, mergeEnd);

    myMerge(begin, middle, end, mergeBegin, mergeEnd);
  }
  break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Timer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Timer::Timer() :
  _start(std::chrono::system_clock::time_point::min())
{
}

void
Timer::Clear()
{
  _start = std::chrono::system_clock::time_point::min();
}

bool
Timer::IsStarted() const
{
  return (_start.time_since_epoch() != std::chrono::system_clock::duration(0));
}

void
Timer::Start()
{
  _start = std::chrono::system_clock::now();
}

unsigned long
Timer::GetMs() const
{
  if (IsStarted())
  {
    std::chrono::system_clock::duration diff = std::chrono::system_clock::now() - _start;
    return (unsigned)(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class JoinThreads
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JoinThreads::JoinThreads(std::vector<std::thread>& threads) :
  _threads(threads)
{
}

JoinThreads::~JoinThreads()
{
  Join();
}

void
JoinThreads::Join()
{
  for (std::thread& thread : _threads)
  {
    if (thread.joinable())
      thread.join();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class JoinThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JoinThread::JoinThread(std::thread& thread) :
  _thread(thread)
{
}

JoinThread::~JoinThread()
{
  if (_thread.joinable())
    _thread.join();
}
