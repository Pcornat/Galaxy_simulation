#include "star.h"
#include "utils.h"
#include "block.h"

// Construit une étoile à des coordonnées aléatoires dans la zone

Star::Star(const double &initial_speed, const double &area, const double &step, const double &galaxy_thickness) {
	is_alive = true;
	position = create_spherical((sqrt(random_double(0., 1.)) - 0.5) * area,
								random_double(0., 2. * PI),
								PI * 0.5); // Multiplication plus rapide qu'une division.
	position.z = ((random_double(0., 1.) - 0.5) * (area * galaxy_thickness));
	speed = create_spherical(initial_speed, position.get_phi() + PI * 0.5, PI * 0.5);
	previous_position = position - speed * step;
	acceleration = Vector(0., 0., 0.);
	color = { 0, 0, 0 };
	mass = 0.;
	density = 0.;
	index = 0;
	block_index = 0;
}



// Assignation

/*Star &Star::operator=(const Star &star) {
	if (this != &star) {
		is_alive = star.is_alive;
		previous_position = star.previous_position;
		position = star.position;
		speed = star.speed;
		acceleration = star.acceleration;
		mass = star.mass;
		density = star.density;
		color = star.color;
		index = star.index;
		block_index = star.block_index;
	}
	return *this;
}*/



// Met à jour la position

void Star::update_position(const double &step, bool verlet_integration) {
	if (verlet_integration) {
		Vector temp = position;

		position = 2. * position - previous_position + acceleration * step * step; // Intégration de Verlet
		previous_position = temp;
	} else
		position += speed * step; // Méthode d'Euler
}



// Met à jour la vitesse

void Star::update_speed(const double &step, const double &area) {
	speed += acceleration * step;
}



// Met à jour l'accélération et la densité

void Star::update_acceleration_and_density(const double &precision, const Block &block) {
	density = 0.;
	double max_acceleration = 0.0000000005; // Permet de limiter l'erreur due au pas de temps (à régler en fonction du pas de temps)

	acceleration = force_and_density_calculation(precision, *this, block); // Pas de division par la masse de l'étoile (c.f. ligne 122)

	if (acceleration.get_radius() > max_acceleration)
		acceleration = max_acceleration * normalize(acceleration);
}



// Calcule la densité et la force exercée sur une étoile (divisée par la masse de l'étoile pour éviter des calculs inutiles)

Vector force_and_density_calculation(const double &precision, Star &star, const Block &block) {
	Vector force = Vector(0., 0., 0.);
	Vector star_to_mass = (star.position - block.mass_center);
	double distance = get_distance(star.position, block.mass_center);

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
					force += force_and_density_calculation(precision, star, blocks[i]);
			}
		}
	}

	return force;
}



// Met à jour la couleur

void Star::update_color() {
	constexpr int colorT3 = 255 * 3, colorT2 = 255 * 2;
	int color_nb = static_cast<int>(density * 2.);

	if (color_nb > colorT3)
		color_nb = colorT3;

	if (color_nb < 255)
		color = { 0, 0, color_nb };

	else if (color_nb < colorT2)
		color = { 0, color_nb - 255, 255 };

	else
		color = { color_nb - colorT2, 255, 255 };
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
		galaxy.back().color = { 255, 10, 10 };
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.121; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(0.45, 0.8) * SOLAR_MASS;
		galaxy.back().color = { 255, 127, 10 };
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.076; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(0.8, 1.04) * SOLAR_MASS;
		galaxy.back().color = { 255, 255, 10 };
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.030; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(1.04, 1.4) * SOLAR_MASS;
		galaxy.back().color = { 255, 255, 127 };
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.006; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(1.4, 2.1) * SOLAR_MASS;
		galaxy.back().color = { 255, 255, 255 };
		galaxy.back().index = galaxy.size() - 1;
	}

	for (int i = 0; i <= stars_number * 0.0013; ++i) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().mass = random_double(2.1, 16) * SOLAR_MASS;
		galaxy.back().color = { 50, 255, 255 };
		galaxy.back().index = galaxy.size() - 1;
	}

	if (is_black_hole) {
		galaxy.emplace_back(initial_speed, area, step, galaxy_thickness);
		galaxy.back().position = Vector(0., 0., 0.);
		galaxy.back().speed = Vector(0., 0., 0.);
		galaxy.back().mass = black_hole_mass * SOLAR_MASS;
		galaxy.back().color = { 0, 0, 0 };
		galaxy.back().index = galaxy.size() - 1;
	}
	galaxy.shrink_to_fit();
}