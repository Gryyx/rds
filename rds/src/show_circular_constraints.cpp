#include "circular_constraints.hpp"
#include "gui.hpp"
#include "vw_limits.hpp"
#include "geometry.hpp"

#include <vector>
#include <cmath>

using Geometry2D::Vec2;
using Geometry2D::HalfPlane2;
using Geometry2D::CircularCorrectionLowerPoints;

int main()
{
	VWDiamond lim(-1.f, 1.5, 1.0, 0.25);
	float y_p = -0.5;

	CircularCorrectionLowerPoints circular_correction(lim, y_p);

	float width = 2.f*std::max(std::max(std::abs(lim.v_max), std::abs(lim.v_min)),
		lim.w_abs_max*std::abs(y_p));
	GUI gui("Circular Constraints", width);
	std::vector<HalfPlane2> all_constraints = circular_correction.getConstraintsVelocityLimits();
	gui.halfplanes = &all_constraints;
	std::vector<Vec2> origin(1);
	gui.points = &origin;

	float theta = 0.f;
	Vec2 n_0(1.f, 1.f);

	while (gui.update() == 0)
	{
		theta += 0.001f;
		float rot[] = {std::cos(theta), -std::sin(theta),
			std::sin(theta), std::cos(theta)};
		Vec2 n_rot(rot[0]*n_0.x + rot[1]*n_0.y, rot[2]*n_0.x + rot[3]*n_0.y);
		HalfPlane2 constraint(n_rot, 0.25f);
		float tau = 1.5f;
		std::vector<HalfPlane2> circular_constraints;
		circular_correction.createCircularConstraints(constraint, tau, &circular_constraints);
		all_constraints.resize(4);
		all_constraints.push_back(constraint);
		for (auto& h : circular_constraints)
			all_constraints.push_back(h);
	}
}