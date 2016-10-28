#pragma once

#include <vector>
#include <chrono>
#include <iosfwd>

namespace std
{
  class thread;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct Point_T
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct Point_T
{
  T x;
  T y;

  constexpr Point_T() noexcept : x(), y() {}
  constexpr Point_T(T _x, T _y) noexcept : x(_x), y(_y) {}
};

typedef Point_T<int16_t> Point;

typedef std::vector<Point> VectorOfPoint;

inline bool lessX(const Point& pt1, const Point& pt2)
{
  return (pt1.x < pt2.x);
}

inline bool lessY(const Point& pt1, const Point& pt2)
{
  return (pt1.y < pt2.y);
}

void ReadInputTextData(std::istream& inputStream, /*out*/VectorOfPoint& inputData);
void ReadNumberOfPoits(std::istream& inputStream, /*out*/size_t& pointCount);
void ReadDataBundle(std::istream& inputStream, size_t bundleSize, VectorOfPoint::iterator current, /*out*/size_t& readPointCount);

bool operator ==(const VectorOfPoint& left, const VectorOfPoint& right);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Divide-and-conquer sorting functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void myMerge(VectorOfPoint::iterator begin, const VectorOfPoint::const_iterator middle,
             const VectorOfPoint::const_iterator end, VectorOfPoint& mergedAux);

void mySort(VectorOfPoint::iterator begin, VectorOfPoint::iterator end, VectorOfPoint& mergedAux);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Timer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Timer
{
public:
  Timer();

  void Clear();

  bool IsStarted() const;

  void Start();

  unsigned long GetMs() const;

private:
  std::chrono::system_clock::time_point _start;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function timeFuncInvocation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class F, class... P>
auto timeFuncInvocation(F&& func, P&&... params)
{
  Timer timer;
  timer.Start();
  std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
  unsigned long duration = timer.GetMs();
  return duration;
};

// auto timeFuncInvocation =
//   [](auto&& func, auto&&... params)
//   {
//     Timer timer;
//     timer.Start();
//     std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
//     unsigned long duration = timer.GetMs();
//     return duration;
//   };


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class JoinThreads
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class JoinThreads
{
public:
  explicit JoinThreads(std::vector<std::thread>& threads);

  ~JoinThreads();

  void Join();

private:
  std::vector<std::thread>& _threads;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class JoinThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class JoinThread
{
public:
  explicit JoinThread(std::thread& thread);
  ~JoinThread();

  JoinThread(const JoinThread&) = delete;
  JoinThread& operator=(const JoinThread&) = delete;

private:
  std::thread& _thread;
};
