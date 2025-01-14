#ifndef  WINDOW_HPP
#define  WINDOW_HPP

#include "geometry.hpp"
#include "capsule.hpp"
#include <SDL2/SDL.h>
#include <vector>

// this class allows multiple windows,
// but not updating them in different threads.
class Window
{
public:
	Window(const char* name, float screen_size_in_distance_units,
		int screen_size_in_pixels, float frame_rate_in_Hz, bool* is_good);
	~Window();

	struct sdlColor
	{
		sdlColor() : r(255), g(255), b(255) {};
		Uint8 r,g,b;
	};

	// returns the delay to the next frame in minutes,
	// after waiting for this very same delay
	float delayUntilNextFrameInMinutes();

	// returns true if the window has been closed, false otherwise
	bool render(const std::vector<Geometry2D::HalfPlane2>* half_planes,
		const std::vector<Geometry2D::Vec2>* points,
		const std::vector<sdlColor>& points_colors,
		const std::vector<AdditionalPrimitives2D::Circle>* circles,
		const std::vector<AdditionalPrimitives2D::Arrow>* arrows,
		const std::vector<Window::sdlColor>& arrows_colors,
		const std::vector<Window::sdlColor>& circles_colors = std::vector<Window::sdlColor>(0),
		const std::vector<AdditionalPrimitives2D::Polygon>* polygons_ptr = 0,
		const std::vector<Geometry2D::Capsule>* capsules_ptr = 0);

	const float screenSizeInDistanceUnits;
	const int screenSizeInPixels;
	const float pixelsPerDistanceUnit;
	const float frameRateInHz;

	bool render_feasible_region;

private:
	// call it once before rendering to a new frame,
	// returns true if the sdl window has been closed
	bool sdlCheck();

	class Pixel
	{
	public:
		Pixel();
		Pixel(int i, int j);
		int i, j;
	};
	Pixel pointToPixel(const Geometry2D::Vec2& point) const;
	Pixel unitVectorToPixelDirection(const Geometry2D::Vec2& unit_vector);
	struct BoundingBox
	{
		Pixel lower_bounds;
		Pixel upper_bounds;
	};
	BoundingBox circleToBoundingBox(const Geometry2D::Vec2& center, float radius);
	Geometry2D::Vec2 pixelToPoint(const Pixel& p) const;

	void renderCircle(const Geometry2D::Vec2& center, float r_outer, float r_inner, const Window::sdlColor& color = Window::sdlColor());
	void renderHalfPlanes(const std::vector<Geometry2D::HalfPlane2>& half_planes);
	void renderBoundaryLine(const Geometry2D::HalfPlane2& half_plane, int pixel_shift = 0);
	void renderArrows(const std::vector<AdditionalPrimitives2D::Arrow>& arrows,
		const std::vector<Window::sdlColor>& arrows_colors);

	void renderPolygons(const std::vector<AdditionalPrimitives2D::Polygon>& polygons);
	void renderCutCircle(const Geometry2D::Vec2& center, float r_outer, float r_inner,
		const Geometry2D::HalfPlane2& h_cut, const Window::sdlColor& color);
	void renderCapsules(const std::vector<Geometry2D::Capsule>& capsules);

	static void killSdlWindow(int window_creation_number);
	struct SDLPointers
	{
		SDL_Window* window;
		SDL_Renderer* renderer;
	};
	static std::vector<SDLPointers> allSdlPointers;
	static bool sdlIsInitialized;

	const int sdlWindowCreationNumber;

	float halfplanes_areas_time, halfplanes_borders_time, points_time, circles_time, arrows_time;
	long long n_frames;
	/*
	class HalfPlaneRenderer
	{
	public:
		HalfPlaneRenderer(const Window& win,
			const std::vector<Geometry2D::HalfPlane2>& halfplanes);
		~HalfPlaneRenderer();
		void render(SDL_Renderer* renderer);
	private:
		struct BinaryPoint
		{
			Geometry2D::Vec2 point;
			bool feasible;
		};
		Geometry2D::Vec2 lower_left_corner_bounding_box;
		Geometry2D::Vec2 upper_left_corner_bounding_box;
		Geometry2D::Vec2 lower_right_corner_bounding_box;
		Geometry2D::Vec2 upper_right_corner_bounding_box;
		std::vector<BinaryPoint> binary_points;
		SDL_Point* infeasible_points;
		int count_infeasible;
	};*/

	struct HalfPlaneRenderer
	{
		HalfPlaneRenderer(const Window& win);
		~HalfPlaneRenderer();
		void render(SDL_Renderer* renderer, const std::vector<Geometry2D::HalfPlane2>& half_planes);
		const Window& win;
		SDL_Point* infeasible_points;
		int count_infeasible;
		SDL_Point* close_to_infeasible_points;
		int count_close_to_infeasible;
		float closeness_threshold;

		void divideAndConquerRendering(SDL_Renderer* renderer, const std::vector<Geometry2D::HalfPlane2>& half_planes);
	private:
		void subdivideAndRender(SDL_Renderer* renderer, const std::vector<Geometry2D::HalfPlane2>& half_planes,
			int n_sub_divide, float lower_x, float upper_x, float lower_y, float upper_y);
		int pointFeasibility(const Geometry2D::Vec2& point, const std::vector<Geometry2D::HalfPlane2>& half_planes);
		int boxFeasibility(float lower_x, float upper_x, float lower_y, float upper_y, const std::vector<Geometry2D::HalfPlane2>& half_planes);
		void renderBox(SDL_Renderer* renderer, bool feasible, float lower_x, float upper_x, float lower_y, float upper_y);
		void createPixelRectangle(float lower_x, float upper_x, float lower_y, float upper_y, SDL_Rect* rectangle);
	};

	HalfPlaneRenderer half_plane_renderer;

	/*struct HierarchicalHalfplaneRenderer
	{
		HalfPlaneRenderer(const Window& win);
		~HalfPlaneRenderer();
		void render(SDL_Renderer* renderer, const std::vector<Geometry2D::HalfPlane2>& half_planes);
		const Window& win;
		SDL_Point* infeasible_points;

		struct AABB
		{
			Geometry2D::Vec2 lower_left_corner;
			Geometry2D::Vec2 upper_left_corner;
			Geometry2D::Vec2 lower_right_corner;
			Geometry2D::Vec2 upper_right_corner;
		};
	private:
		int feasibility(const Geometry2D::Vec2& point, const Geometry2D::HalfPlane2& h);
		int feasibility(const AABB& box, const Geometry2D::HalfPlane2& h);
	};*/
};

#endif