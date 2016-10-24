#include "stdafx.h"
#include "CommonCode.h"
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
Timer::GetMs()
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
