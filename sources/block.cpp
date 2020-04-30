#include "block.h"
#include <array>
#include <functional>

std::array<Star::range, 8> set_octree(const Star::range &stars, const glm::dvec3 &pivot) {
	const std::array<std::function<bool(const Star &star)>, 3> testStarAxis{
			[pivot](const Star &star) { return star.position.x < pivot.x; }, // 3 double c'est plus lourd qu'une référence.
			[pivot](const Star &star) { return star.position.y < pivot.y; },
			[pivot](const Star &star) { return star.position.z < pivot.z; }
	};

	std::array<Star::range, 8> result;
	std::size_t iPart = 0;
	auto itX = std::partition(stars.begin, stars.end, testStarAxis[0]);
	auto xParts = std::array{ Star::range{ stars.begin, itX }, Star::range{ itX, stars.end }};

	for (auto &part : xParts) {
		auto itY = std::partition(part.begin, part.end, testStarAxis[1]);
		auto yParts = std::array{ Star::range{ part.begin, itY }, Star::range{ itY, part.end }};

		for (auto &part : yParts) {
			auto itZ = std::partition(part.begin, part.end, testStarAxis[2]);
			result[iPart++] = Star::range{ part.begin, itZ };
			result[iPart++] = Star::range{ itZ, part.end };
		}
	}

	return result;
}


// Met à jour la masse et le centre de gravit� de chaque blocks

void Block::update_mass_center_and_mass(const Star::range &galaxy) {
	mass_center = { 0., 0., 0. };
	mass = 0.;

	for (auto it = galaxy.begin; it != galaxy.end; ++it) {
		mass_center += it->position * it->mass;
		mass += it->mass;
	}
	mass_center = mass_center / mass;
}

// Divise un bloc en 8 plus petits

void Block::divide(const Star::range &stars) {
	// pas d'etoile
	if (stars.begin == stars.end) {
		contains = stars.begin; // pas tr�s utile, permet de clear la memoire de array<Block, 8> si c'�tait sa valeur pr�c�dente
		nb_stars = 0;
		mass = 0.;
		mass_center = { 0., 0., 0. };
		as_children = false;
		// une étoile
	} else if (std::next(stars.begin) == stars.end) {
		contains = stars.begin;
		nb_stars = 1;
		mass = stars.begin->mass;
		mass_center = stars.begin->position;
		as_children = false;
	} else {
		if (contains.index() != 1)
			contains = std::vector<Block>(8);

		nb_stars = std::distance(stars.begin, stars.end);
		as_children = true;

		Block block;

		block.as_parents = true;
		block.set_size(halfsize);

		const glm::dvec3 posis[] = {
				{ position.x - size / 4., position.y - size / 4., position.z - size / 4. },
				{ position.x - size / 4., position.y - size / 4., position.z + size / 4. },
				{ position.x - size / 4., position.y + size / 4., position.z - size / 4. },
				{ position.x - size / 4., position.y + size / 4., position.z + size / 4. },
				{ position.x + size / 4., position.y - size / 4., position.z - size / 4. },
				{ position.x + size / 4., position.y - size / 4., position.z + size / 4. },
				{ position.x + size / 4., position.y + size / 4., position.z - size / 4. },
				{ position.x + size / 4., position.y + size / 4., position.z + size / 4. }
		};

		auto &myblocks = std::get<1>(contains);
		const auto partitions_stars = set_octree(stars, position);
		double new_mass = 0.;
		auto new_mass_center = glm::dvec3(0., 0., 0.);
		std::size_t i_add = 0;

		for (std::size_t ibloc = 0; ibloc < 8; ++ibloc) {
			myblocks[ibloc] = block;
			myblocks[ibloc].position = posis[ibloc];
			myblocks[ibloc].divide(partitions_stars[ibloc]);

			if (myblocks[ibloc].nb_stars > 0) {
				new_mass += myblocks[ibloc].mass;
				new_mass_center += myblocks[ibloc].mass_center * myblocks[ibloc].mass;
				++i_add;
			}
		}

		mass = new_mass;
		mass_center = new_mass_center / new_mass;
	}
}



// Met � jour la taille du block

void Block::set_size(const double &size) {
	this->size = size;
	this->halfsize = size * 0.5;
}



// Dit si l'�toile est dans le bloc

bool is_in(const Block &block, const Star &star) {
	return (block.position.x + block.size * 0.5 > star.position.x and block.position.x - block.size * 0.5 < star.position.x
			and block.position.y + block.size * 0.5 > star.position.y and block.position.y - block.size * 0.5 < star.position.y
			and block.position.z + block.size * 0.5 > star.position.z and block.position.z - block.size * 0.5 < star.position.z);
}



// G�n�re les blocs

void create_blocks(const double &area, Block &block, Star::range &alive_galaxy) {
	block.set_size(area * 3.);
	block.divide(alive_galaxy);
}