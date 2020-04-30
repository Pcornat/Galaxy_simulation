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

inline int random_int(const int min, const int max) {
	return rand() % (max - min) + min;
}

/**
 * \brief Donne un double aléatoire entre deux bornes
 * \param min
 * \param max
 * \return
 */
inline double random_double(const double min, const double max) {
	return (double(rand()) / double(RAND_MAX)) * (max - min) + min;
}


#endif