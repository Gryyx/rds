#ifndef RDS_5_ORCA_SIMULATOR
#define RDS_5_ORCA_SIMULATOR

#include "geometry.hpp"
#include "rds_5_agent.hpp"
#include "crowd_trajectory.hpp"
#include <RVO.h>
#include <vector>

struct StaticCircle
{
	StaticCircle(const Geometry2D::Vec2& position, float radius, float max_error);
	const AdditionalPrimitives2D::Circle circle;
	const AdditionalPrimitives2D::Polygon polygon;
	const std::vector<RVO::Vector2> rvo_polygon;
	bool robot_collision;
};

struct RdsOrcaSimulator
{
	RdsOrcaSimulator(const Geometry2D::Vec2& position, float orientation,
		const RDS5CapsuleConfiguration& config, const Geometry2D::Vec2& reference_point_velocity, bool orca_orca = false);

	virtual ~RdsOrcaSimulator() { }

	unsigned int addPedestrian(const Geometry2D::Vec2& position, const Geometry2D::Vec2& velocity);

	void setRobotProperties(const Geometry2D::Vec2& position, float orientation,
		const RDS5CapsuleConfiguration& config, const Geometry2D::Vec2& reference_point_velocity);

	void step(float dt);

	const std::vector<MovingCircle>& getPedestrians() const { return m_pedestrians; }

	const RDS5CapsuleAgent& getRobot() const { return m_robot; }

	const Geometry2D::BoundingCircles& getBoundingCirclesRobot() const { return m_bounding_circles_robot; }

	float getTime() const { return m_time; }

	const std::vector<StaticCircle>& getStaticObstacles() const { return m_static_obstacles; }

	void checkRobotCollisions();

	void addStaticObstacle(const Geometry2D::Vec2& position, float radius, bool skip_rvo_simulator = false);

	const std::vector<bool>& getRobotCollisions() const { return m_robot_collisions; }

	AdditionalPrimitives2D::Circle getOrcaOrcaCircle() const;

	void implementORCA(bool use_p_ref) { m_robot.ORCA_implementation = true; m_robot.ORCA_use_p_ref = use_p_ref; }

	void useDefaultNominalCommand(const Geometry2D::Vec2& v_p_ref) { m_robot.use_default_nominal_command = true;
		m_robot.default_v_nominal = v_p_ref; }
	
	void useORCASolver() {m_robot.ORCA_solver = true; }

protected:
	virtual Geometry2D::Vec2 getRobotNominalVelocity();

	virtual RVO::Vector2 getPedestrianNominalVelocity(unsigned int i);
protected:
	RVO::RVOSimulator m_rvo_simulator;
	std::vector<MovingCircle> m_pedestrians;
	RDS5CapsuleAgent m_robot;
	Geometry2D::BoundingCircles m_bounding_circles_robot;
	std::vector<StaticCircle> m_static_obstacles;
	double m_time;
	std::vector<bool> m_robot_collisions;
public:
	const float m_orca_time_horizon;
	const float m_orca_distance_margin;
	const float m_pedestrian_radius;
	const float m_pedestrian_v_max;
	const bool m_orca_orca;
	bool m_robot_avoids;
	bool m_ignore_orca_circle;
	Geometry2D::Vec2 m_previous_robot_position;
};

struct CrowdRdsOrcaSimulator : public RdsOrcaSimulator
{
	CrowdRdsOrcaSimulator(const RDS5CapsuleConfiguration& config,
		const CrowdTrajectory& crowd_trajectory, unsigned int robot_leader_index,
		bool orca_orca = false);

	const CrowdTrajectory m_crowd_trajectory;

	void addPedestrian(unsigned int crowd_pedestrian_index);

	Geometry2D::Vec2 getPedestrianPosition(unsigned int i) const;

	unsigned int getNumberOfPedestrians() const { return m_pedestrians.size(); }

	Geometry2D::Vec2 getPedestrianNominalPosition(unsigned int i) const;

	Geometry2D::Vec2 getRobotNominalPosition() const;

	const std::vector<unsigned int>& getPedestrianIndices() const { return m_crowd_pedestrian_indices; }

	const unsigned int m_robot_leader_index;

	void disableAvoidanceForRecentlyAddedPedestrian();
protected:
	virtual RVO::Vector2 getPedestrianNominalVelocity(unsigned int i);

	virtual Geometry2D::Vec2 getRobotNominalVelocity();

	void updateIgnoreInformation();

	std::vector<unsigned int> m_crowd_pedestrian_indices;
	std::vector<bool> m_pedestrians_avoidance;
	std::vector<unsigned int> m_pedestrians_orca_no;
};

/*struct CrowdOrcaOrcaSimulator
{
	CrowdOrcaOrcaSimulator(const CrowdRdsOrcaSimulator& ref_sim);
};*/

#endif