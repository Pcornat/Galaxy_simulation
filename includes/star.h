#ifndef STAR_H
#define STAR_H

#include "vector.h"
#include "utils.h"
#include <vector>
#include <array>

class Block;

/**
 * \class Star
 * \brief Définit une étoile.
 */
class Star {

public:

	using container = std::vector<Star>; // Plus rapide : contigüe en mémoire.
//	template<int N>
//	using container = std::array<Star, N>;

	struct range {
		container::iterator begin;
		container::iterator end;
	};

	//! Position d'avant
	glm::dvec3 previous_position{ 0, 0, 0 };

	//! Position
	glm::dvec3 position{ 0, 0, 0 };

	//! La vitesse
	glm::dvec3 speed{ 0, 0, 0 };

	//! L'accélération
	glm::dvec3 acceleration{ 0, 0, 0 };

	//! Masse en kilogramme
	double mass{ 0 };

	//! Densité autour de l'étoile
	double density{ 0 };

	//! Indice de l'étoile
	std::size_t index{ 0 };

	//! Indice du bloc
	std::size_t block_index{ 0 };

	//! Flag pour la prise en compte de l'étoile.
	bool is_alive{ false };

	Star() = default;

	~Star() = default; // Pas virtuel pour le rendre utilisable en constexpr

	/**
	 * \brief Construit une étoile à des coordonnées aléatoires dans la zone
	 * \todo Le transformer en constexpr. Ou faire un constructeur vraiment constexpr avec un paramètre supplémentaire.
	 * \param initial_speed
	 * \param area
	 * \param step
	 * \param galaxy_thickness
	 */
	Star(const double initial_speed, const double area, const double step, const double galaxy_thickness);

	Star(const Star &star) = default;

	Star(Star &&star) = default;

	Star &operator=(const Star &star) = default;

	Star &operator=(Star &&star) = default;

	/**
	 * \brief Met à jour la position
	 * \param step
	 * \param verlet_integration
	 */
	void update_position(const double &step, bool verlet_integration);

	/**
	 * \brief Met à jour la vitesse
	 * \param step
	 * \param area
	 */
	void update_speed(const double &step, const double &area);

	/**
	 * \brief Met à jour l'accélération et la densité
	 * \param precision
	 * \param block
	 */
	void update_acceleration_and_density(const double &precision, const Block &block);

};

glm::dvec3 force_and_density_calculation(const double &precision, Star &star, const Block &block);

void initialize_galaxy(Star::container &galaxy,
					   int stars_number,
					   const double &area,
					   const double &initial_speed,
					   const double &step,
					   bool is_black_hole,
					   const double &black_hole_mass,
					   const double &galaxy_thickness);

template<int N>
std::array<Star, N> initialize_galaxy(const int stars_number,
									  const double area,
									  const double initial_speed,
									  const double step,
									  const bool is_black_hole,
									  const double black_hole_mass,
									  const double galaxy_thickness) {
	std::array<Star, N> galaxy;
	std::size_t counter = 0;
	for (int i = 0; i <= stars_number * 0.764; ++i, ++counter) {
		galaxy[counter] = Star(initial_speed, area, step, galaxy_thickness);
		galaxy[counter].mass = random_double(0.08, 0.45) * SOLAR_MASS;
		galaxy[counter].index = counter == 0 ? 0 : counter - 1;
	}

	for (int i = 0; i <= stars_number * 0.121; ++i, ++counter) {
		galaxy[counter] = Star(initial_speed, area, step, galaxy_thickness);
		galaxy[counter].mass = random_double(0.45, 0.8) * SOLAR_MASS;
		galaxy[counter].index = counter - 1;
	}

	for (int i = 0; i <= stars_number * 0.076; ++i, ++counter) {
		galaxy[counter] = Star(initial_speed, area, step, galaxy_thickness);
		galaxy[counter].mass = random_double(0.8, 1.04) * SOLAR_MASS;
		galaxy[counter].index = counter - 1;
	}

	for (int i = 0; i <= stars_number * 0.030; ++i, ++counter) {
		galaxy[counter] = Star(initial_speed, area, step, galaxy_thickness);
		galaxy[counter].mass = random_double(1.04, 1.4) * SOLAR_MASS;
		galaxy[counter].index = counter - 1;
	}

	for (int i = 0; i <= stars_number * 0.006; ++i, ++counter) {
		galaxy[counter] = Star(initial_speed, area, step, galaxy_thickness);
		galaxy[counter].mass = random_double(1.4, 2.1) * SOLAR_MASS;
		galaxy[counter].index = counter - 1;
	}

	for (int i = 0; i <= stars_number * 0.0013; ++i, ++counter) {
		galaxy[counter] = Star(initial_speed, area, step, galaxy_thickness);
		galaxy[counter].mass = random_double(2.1, 16) * SOLAR_MASS;
		galaxy[counter].index = counter - 1;
	}
	++counter;
	if (is_black_hole) {
		galaxy[counter] = Star(initial_speed, area, step, galaxy_thickness);
		galaxy[counter].position = { 0., 0., 0. };
		galaxy[counter].speed = { 0., 0., 0. };
		galaxy[counter].mass = black_hole_mass * SOLAR_MASS;
		galaxy[counter].index = counter - 1;
	}
	return galaxy;
}

#endif