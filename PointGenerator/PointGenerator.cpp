// PointGenerator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdint.h>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>

typedef std::vector<int32_t> VectorOfInt32;
typedef std::set<int32_t> SetOfInt32;
typedef std::vector<std::pair<int32_t, int32_t> > VectorOfPairsInt32;
typedef std::set<std::pair<int32_t, int32_t> > SetOfPairsInt32;


int32_t
RoughlyRandomInRange(int32_t leftBoundary, int32_t rightBoundary)
{
  if ((rightBoundary - leftBoundary) > RAND_MAX)
    throw std::out_of_range("Current simple random generator does not support ranges with '> RAND_MAX' numbers");

  return leftBoundary + (std::rand() % (int32_t)(rightBoundary - leftBoundary + 1));
}

int32_t
RandomInRange(int32_t leftBoundary, int32_t rightBoundary)
{
  // Generate random numbers in the half-closed interval [leftBoundary, rightBoundary).
  return static_cast<int32_t>((double)std::rand() / (RAND_MAX + 1) * (rightBoundary - leftBoundary) + leftBoundary);
}

void
GeneratePoints(int32_t leftBoundary, int32_t rightBoundary, size_t numberOfPoints, VectorOfInt32& generatedPoints)
{
  if (leftBoundary >= rightBoundary)
    throw std::range_error("Input data error: leftBoundary >= rightBoundary");

  generatedPoints.clear();
  generatedPoints.reserve(numberOfPoints);

  for (size_t index = 0; index < numberOfPoints; index++)
    generatedPoints.push_back(RandomInRange(leftBoundary, rightBoundary));
}

void
WritePointsFile(int32_t leftBoundary, int32_t rightBoundary, size_t numberOfPoints, const std::string& fileName)
{
  VectorOfInt32 xCoordinates;
  VectorOfInt32 yCoordinates;

  GeneratePoints(leftBoundary, rightBoundary, numberOfPoints, xCoordinates);
  GeneratePoints(leftBoundary, rightBoundary, numberOfPoints, yCoordinates);

  // Construct vector of (x, y) pairs
  VectorOfPairsInt32 xyCoordinates;
  xyCoordinates.reserve(xCoordinates.size());
  for (auto itX = xCoordinates.begin(), endX = xCoordinates.end(), itY = yCoordinates.begin(); /* initialization */
       itX != endX; /* condition check */
       ++itX, ++itY /* increment: 2 iterators */)
  {
    xyCoordinates.push_back(std::make_pair(*itX, *itY));
  }

  // Eliminate duplicates
  SetOfPairsInt32 uniquePoints(xyCoordinates.begin(), xyCoordinates.end());

  // Randomize points
  VectorOfPairsInt32 shuffledPoints(uniquePoints.begin(), uniquePoints.end());
  std::random_shuffle(shuffledPoints.begin(), shuffledPoints.end());

  // Write points to file
  std::ofstream outStreamText(fileName);
  outStreamText << uniquePoints.size() << '\n'; // This will help to allocate needed amount of memory at one step on reading.
  for (auto it = shuffledPoints.begin(), end = shuffledPoints.end(); it != end; ++it)
  {
    outStreamText << it->first << ' ' << it->second << '\n';
  }
}

int main()
{
  constexpr size_t numberOfPoints20   =      20;
  constexpr size_t numberOfPoints100  =     100;
  constexpr size_t numberOfPoints500  =     500;
  constexpr size_t numberOfPoints1K   =    1000;
  constexpr size_t numberOfPoints5K   =    5000;
  constexpr size_t numberOfPoints500K =  500000;
  constexpr size_t numberOfPoints1M   = 1000000;
  constexpr size_t numberOfPoints2M   = 2000000;
  constexpr size_t numberOfPoints5M   = 5000000;

  constexpr int32_t leftBoundary15  = -15;
  constexpr int32_t leftBoundary1K  = -1000;
  constexpr int32_t leftBoundary16K = -16000;
  constexpr int32_t leftBoundary32K = -32000;

  constexpr int32_t rightBoundary15  = -leftBoundary15 ;
  constexpr int32_t rightBoundary1K  = -leftBoundary1K ;
  constexpr int32_t rightBoundary16K = -leftBoundary16K;
  constexpr int32_t rightBoundary32K = -leftBoundary32K;

  std::string fileCount20Range15  = "PointsCount20Range15.txt";

  std::string fileCount100Range1K  = "PointsCount100Range1K.txt";
  std::string fileCount100Range16K = "PointsCount100Range16K.txt";

  std::string fileCount500Range1K = "PointsCount500Range1K.txt";
  std::string fileCount1KRange1K  = "PointsCount1KRange1K.txt";

  std::string fileCount5KRange16K = "PointsCount5KRange16K.txt";
  std::string fileCount5KRange32K = "PointsCount5KRange32K.txt";

  std::string fileCount500KRange1K  = "PointsCount500KRange1K.txt";
  std::string fileCount500KRange16K = "PointsCount500KRange16K.txt";
  std::string fileCount500KRange32K = "PointsCount500KRange32K.txt";

  std::string fileCount1MRange1K  = "PointsCount1MRange1K.txt";
  std::string fileCount1MRange16K = "PointsCount1MRange16K.txt";
  std::string fileCount1MRange16K_2 = "PointsCount1MRange16K_2.txt";

  std::string fileCount2MRange1K  = "PointsCount2MRange1K.txt";
  std::string fileCount2MRange16K = "PointsCount2MRange16K.txt";
  std::string fileCount2MRange32K = "PointsCount2MRange32K.txt";

  std::string fileCount5MRange16K = "PointsCount5MRange16K.txt";
  std::string fileCount5MRange32K = "PointsCount5MRange32K.txt";

  WritePointsFile(leftBoundary15,  rightBoundary15,  numberOfPoints20  , fileCount20Range15);

  WritePointsFile(leftBoundary1K,  rightBoundary1K,  numberOfPoints100 , fileCount100Range1K);
  WritePointsFile(leftBoundary1K,  rightBoundary1K,  numberOfPoints500 , fileCount500Range1K);
  WritePointsFile(leftBoundary1K,  rightBoundary1K,  numberOfPoints1K  , fileCount1KRange1K);

  WritePointsFile(leftBoundary1K,  rightBoundary1K,  numberOfPoints500K, fileCount500KRange1K);
  WritePointsFile(leftBoundary16K, rightBoundary16K, numberOfPoints5K  , fileCount5KRange16K);
  WritePointsFile(leftBoundary16K, rightBoundary16K, numberOfPoints500K, fileCount500KRange16K);

  WritePointsFile(leftBoundary32K, rightBoundary32K, numberOfPoints5K  , fileCount5KRange32K);
  WritePointsFile(leftBoundary32K, rightBoundary32K, numberOfPoints500K, fileCount500KRange32K);

  WritePointsFile(leftBoundary16K, rightBoundary16K, numberOfPoints1M, fileCount1MRange16K);
  WritePointsFile(leftBoundary16K, rightBoundary16K, numberOfPoints1M, fileCount1MRange16K_2);
  WritePointsFile(leftBoundary16K, rightBoundary16K, numberOfPoints2M, fileCount2MRange16K);
  WritePointsFile(leftBoundary16K, rightBoundary16K, numberOfPoints5M, fileCount5MRange16K);

  WritePointsFile(leftBoundary32K, rightBoundary32K, numberOfPoints2M, fileCount2MRange32K);
  WritePointsFile(leftBoundary32K, rightBoundary32K, numberOfPoints5M, fileCount5MRange32K);

  return 0;
}

