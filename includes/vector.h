#ifndef VECTOR_H
#define VECTOR_H

#define GLM_FORCE_INLINE
#define GLM_FORCE_SIZE_T_LENGTH

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <gcem.hpp>
#include <boost/math/constants/constants.hpp>

using boost::math::constants::pi;

/**
 * \brief Donne la valeur cartésienne x à partir des coordonnées sphériques (en mètres)
 * \param radius
 * \param phi
 * \param theta
 * \return
 */
constexpr double get_x(const double radius, const double phi, const double theta) {
	return gcem::cos(phi) * gcem::sin(theta) * radius;
}

/**
 * \brief Donne la valeur cartésienne y à partir des coordonnées sphériques (en mètres)
 * \param radius
 * \param phi
 * \param theta
 * \return
 */
constexpr double get_y(const double radius, const double phi, const double theta) {
	return gcem::sin(phi) * gcem::sin(theta) * radius;
}

/**
 * \brief Donne la valeur cartésienne z à partir des coordonnées sphériques (en mètres)
 * \param radius
 * \param phi
 * \param theta
 * \return
 */
constexpr double get_z(const double radius, const double phi, const double theta) {
	return gcem::cos(theta) * radius;
}

/**
 * \brief Construit un vecteur à partir de ses coordonnées sphériques
 * \param radius
 * \param phi
 * \param theta
 * \return
 */
constexpr glm::dvec3 create_spherical(const double radius, const double phi, const double theta) {
	return glm::dvec3(get_x(radius, phi, theta), get_y(radius, phi, theta), get_z(radius, phi, theta));
}

namespace glm {
	[[nodiscard]] constexpr double get_phi(const glm::dvec3 &vecteur) {
		auto vector = vecteur;
		vector.z = 0.;

		if (vector.y > 0)
			return gcem::acos(vector.x / gcem::sqrt([=]() -> double {
				const auto tmp = vector * vector;
				return tmp.x + tmp.y + tmp.z;
			}()));

		if (vector.y < 0)
			return 2 * pi<double>() - gcem::acos(vector.x / gcem::sqrt([=]() -> double {
				const auto tmp = vector * vector;
				return tmp.x + tmp.y + tmp.z;
			}()));
	}
}

#endif