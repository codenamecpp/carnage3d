#pragma once

#include <random>

namespace cxx
{
    // defines pseudo random number generator based on Mersenne Twister 19937 generator
    class randomizer
    {
    public:
        randomizer(unsigned int seed = 0) 
            : mRandomGeneratorEngine(seed)
            , mDistribution()
        {
        }

        // set random seed
        // @param randomSeed: Seed
        inline void set_seed(unsigned int randomSeed) 
        {
            mRandomGeneratorEngine.seed(randomSeed);
        }

        // generate random integer
        // @param maxInt: Max int
        // @param minInt: Min int
        inline int generate_int()
        {
            return mDistribution(mRandomGeneratorEngine);
        }

        inline int generate_int(int maxInt)
        {
            return mDistribution(mRandomGeneratorEngine) % maxInt;
        }

        inline int generate_int(int minInt, int maxInt)
        {
            if (minInt == maxInt)
                return generate_int(maxInt);

            debug_assert(maxInt > minInt);
            return minInt + (mDistribution(mRandomGeneratorEngine) % (maxInt - minInt + 1));
        }

        // generate random float in range [0; 1]
        inline float generate_float()
        {
            return generate_int() / (mDistribution.max() * 1.0f + 1.0f);
        }

    private:
        std::mt19937 mRandomGeneratorEngine;
        std::uniform_int_distribution<> mDistribution;
    };

} // namespace cxx