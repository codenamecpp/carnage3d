#pragma once

#include <random>

namespace cxx
{
    // defines pseudo random number generator based on Mersenne Twister 19937 generator
    class randomizer
    {
    public:
        randomizer(unsigned int seed = 0) 
            : mGeneratorEngine(seed)
        {
        }

        // set random seed
        // @param randomSeed: Seed
        inline void set_seed(unsigned int randomSeed) 
        {
            mGeneratorEngine.seed(randomSeed);
        }        

        // generate random integer
        // @param maxInt: Max int
        // @param minInt: Min int
        inline int generate_int()
        {
            return std::uniform_int_distribution<>(0, INT_MAX)(mGeneratorEngine);
        }

        inline int generate_int(int maxInt)
        {
            debug_assert(maxInt >= 0);
            return std::uniform_int_distribution<>(0, maxInt)(mGeneratorEngine);
        }

        inline int generate_int(int minInt, int maxInt)
        {
            if (minInt == maxInt)
                return generate_int(maxInt);

            debug_assert(maxInt > minInt);
            return std::uniform_int_distribution<>(minInt, maxInt)(mGeneratorEngine);
        }

        inline bool random_chance(int chance)
        {
            int currChance = generate_int(1, 100);
            return (chance >= currChance);
        }

        // generate random float in range [0; 1]
        inline float generate_float()
        {
            return generate_int(INT_MAX) / (INT_MAX * 1.0f);
        }

        // shuffle container elements
        template<typename TContainer>
        inline void shuffle(TContainer& container)
        {
            std::shuffle(std::begin(container), std::end(container), mGeneratorEngine);
        }

    private:
        std::mt19937 mGeneratorEngine;
    };

} // namespace cxx