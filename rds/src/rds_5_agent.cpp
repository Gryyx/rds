#include "rds_5_agent.hpp"
#include "distance_minimizer.hpp"
#include <cmath>

using Geometry2D::Vec2;
using AdditionalPrimitives2D::Circle;
using Geometry2D::HalfPlane2;
using Geometry2D::Capsule;

void RDS5CapsuleAgent::stepEuler(float dt,
	const Vec2& v_nominal_p_ref,
	const std::vector<MovingCircle>& objects)
{
	std::vector<MovingCircle> objects_local;

	getObjectsInLocalFrame(objects, &objects_local);
	Vec2 v_nominal_p_ref_local;
	transformVectorGlobalToLocal(v_nominal_p_ref, &v_nominal_p_ref_local);

	if (use_default_nominal_command)
		v_nominal_p_ref_local = default_v_nominal;
	
	float v_box_half = rds_configuration.linear_acceleration_limit*rds_configuration.dt_cycle;
	float w_box_half = rds_configuration.angular_acceleration_limit*rds_configuration.dt_cycle;
	float previous_v_linear = last_step_p_ref_velocity_local.y;
	float previous_v_angular = -last_step_p_ref_velocity_local.x/rds_configuration.y_p_ref;
	VWBox box_limits(previous_v_linear - v_box_half, previous_v_linear + v_box_half,
		previous_v_angular - w_box_half, previous_v_angular + w_box_half);

	Geometry2D::RDS5 rds_5(rds_configuration.tau, rds_configuration.delta, rds_configuration.y_p_ref,
		box_limits, rds_configuration.vw_diamond_limits);

	rds_5.use_conservative_shift = false;
	rds_5.keep_origin_feasible = false;
	rds_5.no_VO_shift_at_contact = false;
	rds_5.shift_reduction_range = 0.35f;
	rds_5.ORCA_implementation = ORCA_implementation;
	rds_5.ORCA_use_p_ref = ORCA_use_p_ref;
	rds_5.ORCA_solver = ORCA_solver;

	if (v_nominal_p_ref_local.norm() > std::abs(rds_configuration.vw_diamond_limits.v_max))
		v_nominal_p_ref_local = v_nominal_p_ref_local.normalized()*std::abs(rds_configuration.vw_diamond_limits.v_max);

	Vec2 v_corrected_p_ref_local;
	try
	{
		rds_5.computeCorrectedVelocity(rds_configuration.robot_shape, v_nominal_p_ref_local,
			last_step_p_ref_velocity_local, std::vector<MovingCircle>(), objects_local,
			&v_corrected_p_ref_local);
	}
	catch (Geometry2D::DistanceMinimizer::InfeasibilityException e)
	{
		float previous_v_linear = last_step_p_ref_velocity_local.y;
		float previous_v_angular = -last_step_p_ref_velocity_local.x/rds_configuration.y_p_ref;
		float breaking_step_linear = rds_configuration.dt_cycle*rds_configuration.breaking_deceleration_linear;
		float breaking_step_angular = rds_configuration.dt_cycle*rds_configuration.breaking_deceleration_angular;
		float new_v_linear, new_v_angular;
		if (previous_v_linear > 0.f)
			new_v_linear = std::max(0.f, previous_v_linear - breaking_step_linear);
		else
			new_v_linear = std::min(0.f, previous_v_linear + breaking_step_linear);
		if (previous_v_angular > 0.f)
			new_v_angular = std::max(0.f, previous_v_angular - breaking_step_angular);
		else
			new_v_angular = std::min(0.f, previous_v_angular + breaking_step_angular);
		v_corrected_p_ref_local.y = new_v_linear;
		v_corrected_p_ref_local.x = -new_v_angular*rds_configuration.y_p_ref;
	}
	
	float v_linear = v_corrected_p_ref_local.y;
	float v_angular = -v_corrected_p_ref_local.x/rds_configuration.y_p_ref;

	Vec2 robot_local_velocity = Vec2(0.f, v_linear);
	Vec2 robot_global_velocity;
	transformVectorLocalToGlobal(robot_local_velocity, &robot_global_velocity);

	position = position + dt*robot_global_velocity;
	orientation = orientation + dt*v_angular;
	transformVectorLocalToGlobal(v_corrected_p_ref_local, &(this->last_step_p_ref_velocity));
	last_step_p_ref_velocity_local = v_corrected_p_ref_local;

	last_step_nominal_p_ref_velocity_local = v_nominal_p_ref_local;
	constraints = rds_5.constraints;
}

void RDS5CapsuleAgent::getObjectsInLocalFrame(const std::vector<MovingCircle>& objects,
		std::vector<MovingCircle>* objects_local)
{
	float rxx = std::cos(-orientation);
	float rxy = std::sin(-orientation);
	float ryx = -rxy;
	float ryy = rxx;
	objects_local->resize(0);
	for (auto& mc : objects)
	{
		objects_local->push_back(MovingCircle(Circle(Vec2(rxx*(mc.circle.center - position).x + ryx*(mc.circle.center - position).y,
			rxy*(mc.circle.center - position).x + ryy*(mc.circle.center - position).y), mc.circle.radius),
			//Vec2()));
			Vec2(rxx*mc.velocity.x + ryx*mc.velocity.y, rxy*mc.velocity.x + ryy*mc.velocity.y)));
	}
}

void RDS5CapsuleAgent::transformVectorGlobalToLocal(const Vec2& v_global, Vec2* v_local) const
{
	*v_local = Vec2(std::cos(orientation)*v_global.x + std::sin(orientation)*v_global.y,
		-std::sin(orientation)*v_global.x + std::cos(orientation)*v_global.y);
}

void RDS5CapsuleAgent::transformVectorLocalToGlobal(const Vec2& v_local, Vec2* v_global) const
{
	*v_global = Vec2(std::cos(orientation)*v_local.x - std::sin(orientation)*v_local.y,
		+std::sin(orientation)*v_local.x + std::cos(orientation)*v_local.y);
}

void RDS5CapsuleAgent::transformReferencePointVelocityToPointVelocity(const Vec2& p_local,
	const Vec2& v_p_ref_global, Vec2* v_global) const
{
	Vec2 v_p_ref_local;
	transformVectorGlobalToLocal(v_p_ref_global, &v_p_ref_local);
	float v_linear = v_p_ref_local.y;
	float v_angular = -v_p_ref_local.x/rds_configuration.y_p_ref;
	Vec2 v_local = Vec2(0.f, v_linear) + Vec2(-v_angular*p_local.y, v_angular*p_local.x);
	transformVectorLocalToGlobal(v_local, v_global);
}

void RDS5CapsuleAgent::computeTightBoundingCircle(Circle* c) const
{
	float y_center = 0.5*(rds_configuration.robot_shape.center_a().y +
		rds_configuration.robot_shape.center_b().y);
	float radius = 0.5f*(rds_configuration.robot_shape.center_a() -
		rds_configuration.robot_shape.center_b()).norm() + rds_configuration.robot_shape.radius();
	if (ORCA_use_p_ref)
	{
		y_center = rds_configuration.y_p_ref;
		radius = y_center - rds_configuration.robot_shape.center_b().y + rds_configuration.robot_shape.radius();
	}
	Vec2 p_center_local(0.f, y_center);
	Vec2 p_center_global;
	transformVectorLocalToGlobal(p_center_local, &p_center_global);
	p_center_global = p_center_global + position;
	*c = Circle(p_center_global, radius);
}