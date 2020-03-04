#ifndef RDS_3_HPP
#define RDS_3_HPP

#include "geometry.hpp"
#include "capsule.hpp"
#include <vector>

namespace Geometry2D
{
	struct RDS3
	{
		RDS3(float tau, float delta, float v_max);

		void computeCorrectedVelocity(const Capsule& robot_shape, const Vec2& p_ref, const Vec2& v_nominal_p_ref,
			const std::vector<AdditionalPrimitives2D::Circle>& objects, Vec2* v_corrected_p_ref);

		void computeCorrectedVelocity(const AdditionalPrimitives2D::Circle& robot_shape, const Vec2& p_ref,
			const Vec2& v_nominal_p_ref, const std::vector<AdditionalPrimitives2D::Circle>& circle_objects,
			const std::vector<Capsule>& capsule_objects, Vec2* v_corrected_p_ref);

		float tau, delta, v_max, v_max_sqrt_2;

		std::vector<Geometry2D::HalfPlane2> constraints;

	private:
		void generateConstraints(const Capsule& robot_shape, const Vec2& p_ref, const Vec2& v_nominal_p_ref,
			const std::vector<AdditionalPrimitives2D::Circle>& objects, std::vector<HalfPlane2>* constraints);

		void generateConstraints(const AdditionalPrimitives2D::Circle& robot_shape, const Vec2& p_ref,
			const Vec2& v_nominal_p_ref, const std::vector<AdditionalPrimitives2D::Circle>& circle_objects, 
			const std::vector<Capsule>& capsule_objects, std::vector<HalfPlane2>* constraints);


		void generateAndAddConstraint(const Vec2& robot_point, float robot_radius,
			const Vec2& object_point, float object_radius, const Vec2& p_ref,
			const Vec2& v_nominal_p_ref, std::vector<HalfPlane2>* constraints);
		
		void solve(const Vec2& v_nominal, std::vector<HalfPlane2>& constraints, Vec2* v_corrected);
	};
}


#endif