#ifndef SPLOOSHKABOOM_H
#define SPLOOSHKABOOM_H

#include <array>
#include <list>

using std::array;

struct config
{
    double prob;
    bool isVertical[3];
    unsigned coords[3][2];
    unsigned surviving[3];
};

enum outcome
{
    miss,
    hit,
    down
};

class SplooshKaboom
{
public:
    SplooshKaboom();
    const unsigned shipSize[3] = {2, 3, 4};
    const unsigned long nMax = 2 * 8 * 7 * 2 * 8 * 6 * 2 * 8 * 5;
    void generate();
    void select(outcome r);
    const array<unsigned, 2> &suggest();

private:
    array<array<bool, 8>, 8> shotSquares;
    std::list<config> remainingConfigs;
    array<unsigned, 2> suggestion;
    bool check(const array<bool, 3> &isVertical, const array<array<unsigned, 2>, 3> &coords);
    void comp_prob();
};

#endif
