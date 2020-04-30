#ifndef UTILS_H
#define UTILS_H


template<typename float_t>
constexpr float_t const_pow(float_t x, int y) {
	float_t res = x;
	for (int i = 1; i < y; i++)
		res *= x;
	return res;
}

constexpr double LIGHT_YEAR = (9.461 * const_pow<double>(10, 15));    // Année lumière (en mètres)
constexpr double G = 0.00000000006674;                                // Constante gravitationnelle (en newton mètre carré par kilogramme carré)

constexpr double SOLAR_MASS = (1.989 * const_pow<double>(10, 30));    // Masse solaire (en kilogrammes)
constexpr double YEAR = 31536000.;                                    // Année (en secondes)

constexpr auto seed() {
	std::uint64_t shifted = 0;

	for (const auto c : __TIME__) {
		shifted <<= 8;
		shifted |= c;
	}

	return shifted;
}

struct PCG {
	struct pcg32_random_t { std::uint64_t state = 0; std::uint64_t inc = seed(); };
	pcg32_random_t rng;
	using result_type = std::uint32_t;

	constexpr result_type operator()() {
		return pcg32_random_r();
	}

	static result_type constexpr min() {
		return std::numeric_limits<result_type>::min();
	}

	static result_type constexpr max() {
		return std::numeric_limits<result_type>::min();
	}

private:
	constexpr std::uint32_t pcg32_random_r() {
		std::uint64_t oldstate = rng.state;
		// Advance internal state
		rng.state = oldstate * 6364136223846793005ULL + (rng.inc | 1);
		// Calculate output function (XSH RR), uses old state for max ILP
		std::uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
		std::uint32_t rot = oldstate >> 59u;
		return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
	}
};

constexpr auto get_random(int count) {
	PCG pcg;
	while (count > 0) {
		pcg();
		--count;
	}

	return pcg();
}

/**
 * \brief Donne un double aléatoire entre deux bornes
 * \param min
 * \param max
 * \return
 */
constexpr double random_double(const double min, const double max) {
	return get_random(static_cast<int>(max - min + min)) / static_cast<double>(RAND_MAX) * (max - min) + min;
}


#endif