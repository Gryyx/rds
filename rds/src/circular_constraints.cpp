#include "circular_constraints.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

namespace Geometry2D
{
	CircularCorrectionLowerPoints::CircularCorrectionLowerPoints(const VWDiamond& vw_diamond_limits,
		float y_p)
		: vw_diamond_limits(vw_diamond_limits)
		, y_p(y_p)
	{
		corner_lower = Vec2(0.f, vw_diamond_limits.v_min);
		corner_upper = Vec2(0.f, vw_diamond_limits.v_max);
		if (y_p < 0.f)
		{
			corner_left = Vec2(vw_diamond_limits.w_abs_max*y_p, vw_diamond_limits.v_at_w_abs_max);
			corner_right = Vec2(-vw_diamond_limits.w_abs_max*y_p, vw_diamond_limits.v_at_w_abs_max);
		}
		else
		{
			corner_left = Vec2(-vw_diamond_limits.w_abs_max*y_p, vw_diamond_limits.v_at_w_abs_max);
			corner_right = Vec2(vw_diamond_limits.w_abs_max*y_p, vw_diamond_limits.v_at_w_abs_max);
		}
		std::vector<Vec2> diamond_corners = {corner_left, corner_lower, corner_right, corner_upper, corner_left};
		for (int i = 0; i < 4; i++)
		{
			constraints_velocity_limits.push_back(HalfPlane2(Vec2(0.f, 0.f), //p_interior
				diamond_corners[i], diamond_corners[i + 1])); // 2 p_boundary
		}
	}

	void CircularCorrectionLowerPoints::createCircularConstraints(const HalfPlane2& constraint,
		float tau, std::vector<HalfPlane2>* circular_constraints)
	{
		circular_constraints->resize(0);
		if (constraint.getOffset() < 0.f)
			return;
		const Vec2& n = constraint.getNormal();
		if (y_p < 0.f)
		{
			if (n.y < 0.f)
			{
				float omega_extreme = -constraint.getOrigo().x/y_p; //origo could be outside limits
				float alpha = -omega_extreme*tau/2.f;
				float rot[] = {std::cos(alpha), -std::sin(alpha),
					std::sin(alpha), std::cos(alpha)};
				Vec2 n_rot(rot[0]*n.x + rot[1]*n.y, rot[2]*n.x + rot[3]*n.y);
				HalfPlane2 h_rot(n_rot, constraint.getOffset());
				HalfPlane2 h_shift(n, constraint.getOffset()*std::cos(alpha));
				// std::cos(alpha) > 0 follows from |alpha| < pi/2
				if (std::abs(alpha) < M_PI/2.f)
				{
					circular_constraints->push_back(h_shift);
					circular_constraints->push_back(h_rot);
				}
				return;
			}
			else
			{
				float omega_extreme_left = -vw_diamond_limits.w_abs_max;
				float omega_extreme_right = vw_diamond_limits.w_abs_max;
				if (n.x < 0.f)
				{
					Vec2 extremum_left = constraints_velocity_limits[0].intersectBoundaries(constraint);
					if (-extremum_left.x/y_p > omega_extreme_left)
						omega_extreme_left = -extremum_left.x/y_p;
					Vec2 test_point = constraints_velocity_limits[2].intersectBoundaries(constraint);
					if (test_point.y < vw_diamond_limits.v_at_w_abs_max)
					{
						Vec2 extremum_right = constraints_velocity_limits[1].intersectBoundaries(constraint);
						omega_extreme_right = -extremum_right.x/y_p;
					}
				}
				else
				{
					Vec2 extremum_right = constraints_velocity_limits[1].intersectBoundaries(constraint);
					if (-extremum_right.x/y_p < omega_extreme_right)
						omega_extreme_right = -extremum_right.x/y_p;
					Vec2 test_point = constraints_velocity_limits[3].intersectBoundaries(constraint);
					if (test_point.y < vw_diamond_limits.v_at_w_abs_max)
					{
						Vec2 extremum_left = constraints_velocity_limits[0].intersectBoundaries(constraint);
						omega_extreme_left = -extremum_left.x/y_p;
					}
				}
				float alpha_left = -omega_extreme_left*tau/2.f;
				float alpha_right = -omega_extreme_right*tau/2.f;
				float rot_left[] = {std::cos(alpha_left), -std::sin(alpha_left),
					std::sin(alpha_left), std::cos(alpha_left)};				
				float rot_right[] = {std::cos(alpha_right), -std::sin(alpha_right),
					std::sin(alpha_right), std::cos(alpha_right)};
				Vec2 n_rot_left(rot_left[0]*n.x + rot_left[1]*n.y,
					rot_left[2]*n.x + rot_left[3]*n.y);
				Vec2 n_rot_right(rot_right[0]*n.x + rot_right[1]*n.y,
					rot_right[2]*n.x + rot_right[3]*n.y);
				HalfPlane2 h_rot_left(n_rot_left, constraint.getOffset());
				HalfPlane2 h_rot_right(n_rot_right, constraint.getOffset());
				float alpha_abs_max = std::max(alpha_left, -alpha_right);
				HalfPlane2 h_shift(n, constraint.getOffset()*std::cos(alpha_abs_max));
				if (alpha_abs_max < M_PI/2.f)
				{
					circular_constraints->push_back(h_shift);
					circular_constraints->push_back(h_rot_right);
					circular_constraints->push_back(h_rot_left);
				}
				return;
			}
		}
	}

	VWDiamond mirror_diamond_limits(const VWDiamond& l, float y_p)
	{
		if (y_p < 0.f)
			return l;
		return VWDiamond(-l.v_max, -l.v_min, l.w_abs_max, -l.v_at_w_abs_max);
	}

	HalfPlane2 mirror_constraint(const HalfPlane2& h)
	{
		return HalfPlane2(-1.f*h.getNormal(), h.getOffset());
	}

	CircularCorrection::CircularCorrection(const VWDiamond& vw_diamond_limits,
		float y_p)
		: vw_diamond_limits(vw_diamond_limits)
		, y_p(y_p)
		, circular_correction_lower_points(mirror_diamond_limits(vw_diamond_limits, y_p), -std::abs(y_p))
	{
		constraints_velocity_limits = circular_correction_lower_points.getConstraintsVelocityLimits();
		if (y_p >= 0.f)
		{
			for (auto& h : constraints_velocity_limits)
				h = mirror_constraint(h);
		}
	}

	void CircularCorrection::createCircularConstraints(const HalfPlane2& constraint,
		float tau, std::vector<HalfPlane2>* circular_constraints)
	{
		if (y_p < 0.f)
		{
			circular_correction_lower_points.createCircularConstraints(constraint,
				tau, circular_constraints);
		}
		else if (y_p > 0.f)
		{
			circular_correction_lower_points.createCircularConstraints(mirror_constraint(constraint),
				tau, circular_constraints);
			for (auto& h : *circular_constraints)
				h = mirror_constraint(h);
		}
		else
		{
			circular_constraints->resize(0);
			return;
		}
	}
}