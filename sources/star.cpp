#include "star.h"
#include "utils.h"
#include "block.h"
#include <gcem.hpp>

// Construit une étoile à des coordonnées aléatoires dans la zone
Star::Star(const double initial_speed, const double area, const double step, const double galaxy_thickness) {
	is_alive = true;
	position = create_spherical((gcem::sqrt(random_double(0., 1.)) - 0.5) * area,
								random_double(0., 2. * pi<double>()),
								pi<double>() * 0.5); // Multiplication plus rapide qu'une division.
	position.z = ((random_double(0., 1.) - 0.5) * (area * galaxy_thickness));
	speed = create_spherical(initial_speed, glm::get_phi(position) + pi<double>() * 0.5, pi<double>() * 0.5);
	previous_position = position - speed * step;
	acceleration = { 0., 0., 0. };
	mass = 0.;
	density = 0.;
	index = 0;
	block_index = 0;
}

void Star::update_position(const double &step, bool verlet_integration) {
	if (verlet_integration) {
		auto temp = position;

		position = 2. * position - previous_position + acceleration * step * step; // Intégration de Verlet
		previous_position = temp;
	} else
		position += speed * step; // Méthode d'Euler
}

void Star::update_speed(const double &step, const double &area) {
	speed += acceleration * step;
}

void Star::update_acceleration_and_density(const double &precision, const Block &block) {
	density = 0.;
	constexpr double max_acceleration = 0.0000000005; // Permet de limiter l'erreur due au pas de temps (à régler en fonction du pas de temps)

	// Pas de division par la masse de l'étoile (c.f. ligne 122) EDIT : trouver un autre moyen de référence que la ligne de code.
	acceleration = force_and_density_calculation(precision, *this, block); // Fonction récursive… Il faut éviter.

	if (glm::length(acceleration) > max_acceleration)
		acceleration = max_acceleration * normalize(acceleration);
}



// Calcule la densité et la force exercée sur une étoile (divisée par la masse de l'étoile pour éviter des calculs inutiles)

glm::dvec3 force_and_density_calculation(const double &precision, Star &star, const Block &block) {
	glm::dvec3 force(0); // Tous les champs à 0.
	const auto star_to_mass = (star.position - block.mass_center);
	double distance = glm::distance(star.position, block.mass_center);

	if (block.nb_stars == 1) {
		Star::container::iterator itStar = std::get<0>(block.contains);

		if (distance != 0.) {
			double inv_distance = 1. / distance;
			force += (star_to_mass * inv_distance) * (-(G * block.mass) / (distance * distance));
			star.density += (inv_distance / LIGHT_YEAR);
		}
	} else {
		double thema = block.size / distance;

		if (thema < precision) {
			force += (star_to_mass / distance) * (-(G * block.mass) / (distance * distance));
			star.density += block.nb_stars / (distance / LIGHT_YEAR);
		} else {
			auto &blocks = std::get<1>(block.contains);
			for (int i = 0; i < 8; ++i) {
				if (blocks[i].nb_stars > 0)
					force += force_and_density_calculation(precision, star, blocks[i]); // WTF ?! PAS DE RÉCURSIF, PERTE DE PERF
			}
		}
	}

	return force;
}

// Initialise la galaxie

void initialize_galaxy(Star::container &galaxy,
					   int stars_number,
					   const double &area,
					   const double &initial_speed,
					   const double &step,
					   bool is_black_hole,
					   const double &black_hole_mass,
					   const double &galaxy_thickness) {
	for (int i = 0; i <= stars_number * 0.764; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(0.08, 0.45) * SOLAR_MASS;
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.121; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(0.45, 0.8) * SOLAR_MASS;
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.076; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(0.8, 1.04) * SOLAR_MASS;
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.030; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(1.04, 1.4) * SOLAR_MASS;
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.006; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(1.4, 2.1) * SOLAR_MASS;
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.0013; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(2.1, 16) * SOLAR_MASS;
		galaxy.back().index = galaxy.size() - 1;
	}

	if (is_black_hole) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().position = { 0., 0., 0. };
		galaxy.back().speed = { 0., 0., 0. };
		galaxy.back().mass = black_hole_mass * SOLAR_MASS;
		galaxy.back().index = galaxy.size() - 1;
	}
	galaxy.shrink_to_fit();
}