#pragma once

#include <vector>
#include <chrono>
#include <thread>
#include <iosfwd>

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

void
ReadInputTextData(std::istream& inputStream, /*out*/VectorOfPoint& inputData);

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

  unsigned long GetMs();

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
