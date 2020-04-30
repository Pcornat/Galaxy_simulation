#include "star.h"
#include "utils.h"
#include "block.h"
#include <atomic>
#include <array>
#include <thread>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>

struct MutexRange {
	Star::range part;
	std::atomic<int> ready = 0;
};

template<size_t N>
[[gnu::always_inline]] inline void make_partitions(std::array<MutexRange, N> &mutparts, Star::range alive_galaxy, const size_t total) {
	const size_t n_per_part = total / N;
	constexpr std::size_t maxLoop = N - 1;
	auto current_it = alive_galaxy.begin, prev_it = alive_galaxy.begin;
	for (size_t i = 0; i < maxLoop; ++i) {
		for (size_t i_it = 0; i_it < n_per_part; ++i_it, ++current_it);

		mutparts[i].part = { prev_it, current_it };
		mutparts[i].ready = 1;

		prev_it = current_it;
	}
	mutparts.back().part = { current_it, alive_galaxy.end };
	mutparts.back().ready = 1;
}

int main() {


	// ------------------------- Paramètres de la simulation -------------------------
	constexpr double area = 1000. * LIGHT_YEAR;                // Taille de la zone d'apparition des étoiles (en années lumière)
	constexpr double galaxy_thickness = 0.05;    // Epaisseur de la galaxie (en "area")

	constexpr int stars_number = 50000;        // Nombre d'étoiles
	constexpr double initial_speed = 10000.;        // Vitesse initiale des d'étoiles (en mètres par seconde)

	constexpr bool is_black_hole = false;        // Présence d'un trou noir
	constexpr double black_hole_mass = 0.;        // Masse du trou noir (en masses solaires)

	constexpr double step = 100000. * YEAR;                // Pas de temps de la simulation (en années de simulation)
	constexpr double precision = 1.;                // Précision du calcul de l'accélération (algorithme de Barnes-Hut)
	constexpr bool verlet_integration = true;    // Utiliser l'intégration de Verlet au lieu de la méthode d'Euler

	constexpr std::size_t maxIter = 1000;

	constexpr std::size_t n_thread = 4; // Le nombre de thread utilisé pour le calcul.



	// -------------------------------------------------------------------------------

/* Inutile -> il vaut mieux laisser en commentaire et laisser à l'utilisateur le contrôle. Tant pis pour lui si ça foire. */
//	if (area < 0.1) area = 0.1;
//	if (galaxy_thickness > 1.) galaxy_thickness = 1.;
//	if (precision < 0.) precision = 0.;
//	if (stars_number < 1.) stars_number = 1.;
//	if (initial_speed < 0.) initial_speed = 0.;
//	if (black_hole_mass < 0.) black_hole_mass = 0.;
//	if (zoom < 1.) zoom = 1.;
//	if (step < 0.) step = 0.;

// On peut le faire dès le début du coup.
//	area *= LIGHT_YEAR;
//	step *= YEAR;

	Star::container galaxy;
	Block block;

	initialize_galaxy(galaxy, stars_number, area, initial_speed, step, is_black_hole, black_hole_mass, galaxy_thickness);
//	constexpr auto galax_ = initialize_galaxy<stars_number>(stars_number,
//															area, initial_speed, step, is_black_hole, black_hole_mass, galaxy_thickness);
//	std::array<Star, stars_number> galax = galax_;

	Star::range alive_galaxy = { galaxy.begin(), galaxy.end() };
	double current_step = 1.;
	bool stop_threads = false;
	const auto update_stars = [&block, precision, verlet_integration, step, area, &stop_threads, &current_step](MutexRange *mutpart) {
		using namespace std::chrono_literals;
		while (mutpart->ready != 1)
			std::this_thread::sleep_for(2ms);

		while (!stop_threads) {
			for (auto it_star = mutpart->part.begin; it_star != mutpart->part.end; ++it_star) // Boucle sur les étoiles de la galaxie
			{
				it_star->update_acceleration_and_density(precision, block);

				if (!verlet_integration)
					it_star->update_speed(step * current_step, area);

				it_star->update_position(step * current_step, verlet_integration);

				if (!is_in(block, *it_star))
					it_star->is_alive = false;
			}

			mutpart->ready = 2;

			while (mutpart->ready != 1 && !stop_threads)
				std::this_thread::sleep_for(2ms);
		}
	};

	std::array<std::thread, n_thread> mythreads;
	std::array<MutexRange, n_thread> mutparts;

	for (std::size_t i = 0; i < mythreads.size(); ++i) {
		mutparts[i].ready = 0;
		mythreads[i] = std::thread(update_stars, &mutparts[i]);
	}

	auto total_galaxy = std::distance(alive_galaxy.begin, alive_galaxy.end);
	auto t0 = std::chrono::steady_clock::now();

	for (size_t i = 0; i < maxIter; ++i) {
		using namespace std::chrono_literals;
		namespace chrono = std::chrono;
		create_blocks(area, block, alive_galaxy);

		make_partitions<n_thread>(mutparts, alive_galaxy, total_galaxy);
		for (auto &mp : mutparts) {
			while (mp.ready != 2)
				std::this_thread::sleep_for(1ms);
		}
		// On dump le pas de la simu :
		{
			std::ofstream output("galaxy_" + std::to_string(i) + ".txt", std::ios::out | std::ios::app | std::ios::trunc);
			for (const auto &item : galaxy) {
				output << item << '\n';
			}
		}
		{
			const auto prev_end = alive_galaxy.end;
			alive_galaxy.end = std::partition(alive_galaxy.begin, alive_galaxy.end, [](const Star &star) { return star.is_alive; });
			total_galaxy -= std::distance(alive_galaxy.end, prev_end);
		}

		auto t1 = chrono::steady_clock::now();
		chrono::duration<double, std::ratio<1, 60>> duree = t1 - t0;
		t0 = t1;
		current_step = duree.count();
	}

	stop_threads = true;
	for (auto &thr : mythreads) {
		thr.join();
	}
	return EXIT_SUCCESS;
}