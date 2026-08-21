//
// Created by penggrin on 22.03.2026.
//

#ifndef VOXELGAME_RANDOM_HPP
#define VOXELGAME_RANDOM_HPP

#include <concepts>
#include <random>

namespace rng {
    constexpr uint64_t hashCoords(const glm::ivec3 pos, uint64_t seed = 0) noexcept {
        uint64_t h = seed;
        h ^= (static_cast<uint64_t>(static_cast<uint32_t>(pos.x)) * 0x1B873593ULL);
        h = (h << 13) | (h >> 51);
        h ^= (static_cast<uint64_t>(static_cast<uint32_t>(pos.y)) * 0x85EBCA6BULL);
        h = (h << 17) | (h >> 47);
        h ^= (static_cast<uint64_t>(static_cast<uint32_t>(pos.z)) * 0x27D4EB2FULL);
        h = (h << 31) | (h >> 33);

        // Final avalanche
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53ULL;
        h ^= h >> 33;
        return h;
    }

    class Generator {
    private:
        uint64_t state_{0};
        uint64_t inc_{1};
    public:
        using result_type = uint32_t;

        constexpr explicit Generator(const uint64_t seed = 0x853c49e6748fea9bULL) {
            setSeed(seed);
        }

        constexpr void setSeed(const uint64_t seed) {
            state_ = seed + 0xda3e39cb94b95bdbULL;
            inc_ = (seed << 1u) | 1u;
            (*this)(); // Warm up
        }

        constexpr result_type operator()() {
            const uint64_t oldState = state_;
            state_ = oldState * 6364136223846793005ULL + inc_;
            const auto xorShifted = static_cast<uint32_t>(((oldState >> 18u) ^ oldState) >> 27u);
            const auto rot = static_cast<uint32_t>(oldState >> 59u);
            return (xorShifted >> rot) | (xorShifted << (~rot + 1u & 31));
        }

        static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
        static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

        // inclusive [from, to]
        template <std::integral T>
        [[nodiscard]] T range(T from, T to) {
            std::uniform_int_distribution<T> dist(from, to);
            return dist(*this);
        }

        // inclusive [0, to]
        template <std::integral T>
        [[nodiscard]] T range(T to) {
            return range<T>(0, to);
        }

        // inclusive [from, to)
        template <std::floating_point T>
        [[nodiscard]] T range(T from, T to) {
            std::uniform_real_distribution<T> dist(from, to);
            return dist(*this);
        }

        // inclusive [0.0, to)
        template <std::floating_point T>
        [[nodiscard]] T range(T to) {
            return range<T>(static_cast<T>(0), to);
        }

        // inclusive [0.0f, 1.0f)
        [[nodiscard]] float randFloat() {
            return range(0.0f, 1.0f);
        }

        // probability=1.0 = 100%
        [[nodiscard]] bool boolean(const double probability = 0.5) {
            std::bernoulli_distribution dist(probability);
            return dist(*this);
        }
    };

    [[nodiscard]] inline Generator& threadLocal() {
        thread_local Generator gen(std::random_device{}());
        return gen;
    }

    template <std::integral T>
    [[nodiscard]] inline T range(T from, T to) {
        return threadLocal().range(from, to);
    }

    template <std::integral T>
    [[nodiscard]] inline T range(T to) {
        return threadLocal().range(to);
    }

    template <std::floating_point T>
    [[nodiscard]] inline T range(T from, T to) {
        return threadLocal().range(from, to);
    }

    template <std::floating_point T>
    [[nodiscard]] inline T range(T to) {
        return threadLocal().range(to);
    }

    [[nodiscard]] inline bool randBool(double probability = 0.5) {
        return threadLocal().boolean(probability);
    }

    [[nodiscard]] inline float randFloat() {
        return threadLocal().randFloat();
    }

    [[nodiscard]] inline float sampleFloat(const glm::ivec3 pos) {
        const uint64_t h = hashCoords(pos);
        return static_cast<float>(h >> 40) * (1.0f / 16777216.0f);
    }

    [[nodiscard]] inline glm::vec2 sampleVec2(const glm::ivec3 pos) {
        const uint64_t h = hashCoords(pos);
        constexpr float scale = 1.0f / 16777216.0f; // 2^24

        const auto x = static_cast<uint32_t>(h);
        const auto y = static_cast<uint32_t>(h >> 32);

        return glm::vec2{
            static_cast<float>(x >> 8) * scale,
            static_cast<float>(y >> 8) * scale
        };
    }

    [[nodiscard]] inline float sampleFloat(const glm::ivec3 pos, const float min, const float max) {
        return glm::mix(min, max, sampleFloat(pos));
    }

    [[nodiscard]] inline glm::vec2 sampleVec2(const glm::ivec3 pos, const float min, const float max) {
        return glm::mix(glm::vec2(min), glm::vec2(max), sampleVec2(pos));
    }

    [[nodiscard]] inline glm::vec2 sampleVec2(const glm::ivec3 pos, glm::vec2 min, glm::vec2 max) {
        return glm::mix(min, max, sampleVec2(pos));
    }
}

#endif //VOXELGAME_RANDOM_HPP