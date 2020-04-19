#include "SplooshKaboom.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <vector>

SplooshKaboom::SplooshKaboom()
{
    generate();
}

bool SplooshKaboom::check(const array<bool, 3> &isVertical,
                          const array<array<unsigned, 2>, 3> &coords)
{
    for (unsigned i = 0; i < 3; ++i)
    {
        for (unsigned j = i + 1; j < 3; ++j)
        {
            /* by the power of underflow, the following just works! :D */
            if (isVertical[i] == isVertical[j])
            {
                if (coords[i][0] == coords[j][0] &&
                    (coords[j][1] - coords[i][1] < shipSize[i] ||
                     coords[i][1] - coords[j][1] < shipSize[j]))
                {
                    return false;
                }
            }
            else
            {
                if (coords[j][0] - coords[i][1] < shipSize[i] &&
                    coords[i][0] - coords[j][1] < shipSize[j])
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void SplooshKaboom::generate()
{
    const unsigned coeff[3] = {7, 6, 5};
    for (unsigned i = 0; i < 8; ++i)
    {
        for (unsigned j = 0; j < 8; ++j)
        {
            shotSquares[i][j] = false;
        }
    }
    for (unsigned long proposal = 0; proposal < nMax; ++proposal)
    {
        array<bool, 3> isVertical;
        array<array<unsigned, 2>, 3> coords;
        unsigned long copy = proposal;
        for (int i = 2; i > -1; --i)
        {
            coords[i][1] = copy % coeff[i];
            copy /= coeff[i];
            coords[i][0] = copy % 8;
            copy /= 8;
            isVertical[i] = copy % 2;
            copy /= 2;
        }
        if (!check(isVertical, coords))
        {
            continue;
        }
        config thisConfig;
        thisConfig.prob = 1.0;
        for (unsigned i = 0; i < 3; ++i)
        {
            thisConfig.isVertical[i] = isVertical[i];
            thisConfig.coords[i][0] = coords[i][0];
            thisConfig.coords[i][1] = coords[i][1];
            thisConfig.surviving[i] = shipSize[i];
        }
        remainingConfigs.push_back(thisConfig);
    }
    comp_prob();
}

void SplooshKaboom::select(outcome r)
{
    unsigned long del = 0;
    std::list<config>::iterator p = remainingConfigs.begin();
    while (p != remainingConfigs.end())
    {
        /* check what ship should have been hit, if any */
        unsigned i = 0;
        while (i < 3)
        {
            bool primary = p->isVertical[i];
            if (p->coords[i][0] == suggestion[primary] &&
                suggestion[!primary] - p->coords[i][1] < shipSize[i])
            {
                break;
            }
            else
            {
                ++i;
            }
        }
        bool good = true;
        switch (r)
        {
        case miss:
            if (i != 3)
            {
                good = false;
            }
            break;
        case hit:
            if (i == 3 || p->surviving[i] == 1)
            {
                good = false;
            }
            break;
        case down:
            if (i == 3 || p->surviving[i] != 1)
            {
                good = false;
            }
            break;
        }
        if (good)
        {
            if (i != 3)
            {
                --p->surviving[i];
            }
            ++p;
        }
        else
        {
            p = remainingConfigs.erase(p);
            ++del;
        }
    }
}

void SplooshKaboom::comp_prob()
{
    std::vector<unsigned long> partialIDs[3];
    for (unsigned i = 0; i < 3; ++i)
    {
        partialIDs[i].reserve(nMax);
    }
    for (config c : remainingConfigs)
    {
        unsigned long id = 0;
        for (unsigned i = 0; i < 3; ++i)
        {
            id |= c.isVertical[i] << 6 + 7 * (2 - i);
            id |= c.coords[i][0] << 3 + 7 * (2 - i);
            id |= c.coords[i][1] << 0 + 7 * (2 - i);
            if (partialIDs[i].empty() || id != partialIDs[i].back())
            {
                partialIDs[i].push_back(id);
            }
        }
    }
    for (std::list<config>::iterator i = remainingConfigs.begin();
         i != remainingConfigs.end(); ++i)
    {
        unsigned long id = 0;
        for (unsigned j = 0; j < 3; ++j)
        {
            id |= i->isVertical[j] << 6 + 7 * (2 - j);
            id |= i->coords[j][0] << 3 + 7 * (2 - j);
            id |= i->coords[j][1] << 7 * (2 - j);
        }
        for (unsigned j = 0; j < 3; ++j)
        {
            std::vector<unsigned long>::iterator a = partialIDs[j].begin();
            std::vector<unsigned long>::iterator b = partialIDs[j].end();
            unsigned long first = (id >> 7 * (3 - j)) << (7 * (3 - j));
            unsigned long last = ((id >> 7 * (3 - j)) + 1) << (7 * (3 - j));
            unsigned long n = std::lower_bound(a, b, last) -
                              std::lower_bound(a, b, first);
            i->prob /= n;
        }
    }
}

const array<unsigned, 2> &SplooshKaboom::suggest()
{
    double board[8][8] = {};
    for (config c : remainingConfigs)
    {
        for (unsigned i = 0; i < 3; ++i)
        {
            for (unsigned j = 0; j < shipSize[i]; ++j)
            {
                unsigned row, column;
                if (c.isVertical[i])
                {
                    column = c.coords[i][0];
                    row = c.coords[i][1] + j;
                }
                else
                {
                    row = c.coords[i][0];
                    column = c.coords[i][1] + j;
                }
                board[row][column] += c.prob / 9.0;
            }
        }
    }
    double max = 0.0;
    for (unsigned i = 0; i < 8; ++i)
    {
        for (unsigned j = 0; j < 8; ++j)
        {
            if (!shotSquares[i][j] && board[i][j] > max)
            {
                suggestion[0] = i;
                suggestion[1] = j;
                max = board[i][j];
            }
        }
    }
    shotSquares[suggestion[0]][suggestion[1]] = true;
    return suggestion;
}
