#include "crowd_trajectory.hpp"
#include <fstream>
#include <string>
#include <iostream>

using Geometry2D::Vec2;

CrowdTrajectory::CrowdTrajectory(const char* data_file_name, float frame_rate, float scaling)
	: m_time_shift(0.f), m_duration(-1.f), m_deceleration_period(0.5f)
{
	std::ifstream data_file(data_file_name);
	float pos_x, pos_y;
	long int frame_i;
	std::string any_word;
	const int n_words_first_line = 6;
	const int n_words_spline_header = 6;
	const int n_words_after_data = 5;

	for (int i = 0; i < n_words_first_line; i++)
		data_file >> any_word;
	while (true)
	{
		std::vector<Knot> spline_data;
		int n_knots;
		if (!(data_file >> n_knots))
			break;
		for (int i = 0; i < n_words_spline_header -1; i++)
			data_file >> any_word;
		for (int j = 0; j < n_knots; j++)
		{
			data_file >> pos_x >> pos_y >> frame_i;
			for (int i = 0; i < n_words_after_data; i++)
				data_file >> any_word;
			spline_data.push_back(Knot(scaling*Vec2(pos_x, pos_y), frame_i/frame_rate));
		}
		m_splines_data.push_back(spline_data);
	}

	for (auto& d : m_splines_data)
	{
		std::vector<double> T, X, Y;
		for (auto& knot : d)
		{
			T.push_back(knot.t);
			X.push_back(knot.p.x);
			Y.push_back(knot.p.y);
		}
		tk::spline sx, sy;
		sx.set_points(T, X);
		sy.set_points(T, Y);
		m_x_splines.push_back(sx);
		m_y_splines.push_back(sy);
	}
}

void CrowdTrajectory::getPedestrianPositionAtTime(unsigned int i, float t,
	Vec2* p) const
{
	if (i >= 666)
	{
		p->x = 100000.f;
		p->y = 100000.f;
		return;
	}
	float weight_trajectory = 1.f;
	float weight_standstill = 0.f;
	if (m_duration > 0.f)
	{
		if (t >= m_duration)
		{
			weight_trajectory = 0.f;
			weight_standstill = 1.f;
		}
		else if (t >= m_duration - m_deceleration_period)
		{
			weight_trajectory = (m_duration - t)/m_deceleration_period;
			weight_standstill = 1.f - weight_trajectory;
		}
	}
	p->x = weight_trajectory*m_x_splines[i](t + m_time_shift) + weight_standstill*m_x_splines[i](m_duration + m_time_shift);
	p->y = weight_trajectory*m_y_splines[i](t + m_time_shift) + weight_standstill*m_y_splines[i](m_duration + m_time_shift);
}

void CrowdTrajectory::getPedestrianVelocityAtTime(unsigned int i, float t,
	Vec2* v) const
{
	if (i >= 666)
	{
		v->x = 0.f;
		v->y = 0.f;
		return;
	}
	float weight_trajectory = 1.f;
	if (m_duration > 0.f)
	{
		if (t >= m_duration)
			weight_trajectory = 0.f;
		else if (t >= m_duration - m_deceleration_period)
			weight_trajectory = (m_duration - t)/m_deceleration_period;
	}
	v->x = weight_trajectory*m_x_splines[i].deriv(1, t + m_time_shift);
	v->y = weight_trajectory*m_y_splines[i].deriv(1, t + m_time_shift);
}

void CrowdTrajectory::addPedestrianTrajectory(const std::vector<Knot>& spline_data)
{
	m_splines_data.push_back(spline_data);

	std::vector<double> T, X, Y;
	for (auto& knot : spline_data)
	{
		T.push_back(knot.t);
		X.push_back(knot.p.x);
		Y.push_back(knot.p.y);
	}
	tk::spline sx, sy;
	sx.set_points(T, X);
	sy.set_points(T, Y);
	m_x_splines.push_back(sx);
	m_y_splines.push_back(sy);
}

void CrowdTrajectory::removePedestrian(unsigned int i)
{
	m_splines_data.erase(m_splines_data.begin() + i);
	m_x_splines.erase(m_x_splines.begin() + i);
	m_y_splines.erase(m_y_splines.begin() + i);
}