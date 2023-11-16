#include <chrono>
#include <iostream>
#include <vector>

#include "aabb_grid.h"

inline v2<float> v2_add(v2<float> a, v2<float> b)
{
	return {a.x + b.x, a.y + b.y};
}

inline v2<float> v2_sub(v2<float> a, v2<float> b)
{
	return {a.x - b.x, a.y - b.y};
}

inline float v2_dot(v2<float> a, v2<float> b)
{
	return a.x * b.x + a.y * b.y;
}

inline v2<float> v2_mul_s(v2<float> a, float b)
{
	a.x *= b;
	a.y *= b;
	return a;
}

inline v2<float> V2(float x, float y)
{
	return {x, y};
}

struct Circle
{
	v2<float> p;
	float r;
};

bool circle_to_circle(Circle a, Circle b, v2<float> *out)
{
	v2 d = v2_sub(b.p, a.p);
	float d2 = v2_dot(d, d);
	float r = a.r + b.r;

	if (d2 < r * r)
	{
		float l = sqrtf(d2);
		v2 n = l != 0 ? v2_mul_s(d, 1.f / l) : V2(0.f, 1.f);
		n = v2_mul_s(n, r - l);
		*out = n;

		return true;
	}

	return false;
}

struct GridCell
{
	int x, y;

	inline GridCell operator-(GridCell const &other) const
	{
		return {x - other.x, y - other.y};
	}
};

inline GridCell abs(GridCell grid_cell)
{
	return {abs(grid_cell.x), abs(grid_cell.y)};
}

GridCell get_cell(Circle c)
{
	const int cell_size = 16;
	return {(int)(c.p.x / cell_size), (int)(c.p.y / cell_size)};
}

void regular_c_test()
{
	std::vector<Circle> entities;
	const float unit_size = 16.f;

	const int min_num_entities = 5000;
	int bounds = (int)ceil(sqrt(min_num_entities) / 2.f);

	for (int x = -bounds; x < bounds; x++)
	{
		for (int y = -bounds; y < bounds; y++)
		{
			const v2 p = v2_mul_s({(float)x, (float)y}, unit_size);
			Circle c = {p, unit_size};
			entities.emplace_back(c);
		}
	}

	v2 n = {};

	auto t1 = std::chrono::high_resolution_clock::now();
	int collision_count = 0;

	for (int i = 0; i < entities.size(); i++)
	{
		auto &a = entities[i];
		auto a_cell = get_cell(a);
		for (int j = i + 1; j < entities.size(); j++)
		{
			auto &b = entities[j];
			auto b_cell = get_cell(b);
			auto relative_pos = abs(a_cell - b_cell);
			if (relative_pos.x <= 1 && relative_pos.y <= 1)
			{
				if (circle_to_circle(a, b, &n))
				{
					a.p = v2_sub(a.p, n);
					b.p = v2_add(b.p, n);

					collision_count++;
				}
			}
		}
	}

	auto t2 = std::chrono::high_resolution_clock::now();
	double frame_time = std::chrono::duration<double>(t2 - t1).count();
	std::cout << "frame_time = " << frame_time << std::endl;
	std::cout << "fps = " << 1.f / frame_time << std::endl;
	std::cout << "size = " << entities.size() << std::endl;
	std::cout << "collisions = " << collision_count << std::endl;
}

void aabb_grid_test()
{
	std::vector<Circle> entities;
	const float unit_size = 16.f;

	const int min_num_entities = 5000;
	int bounds = (int)ceil(sqrt(min_num_entities) / 2.f);

	for (int x = -bounds; x < bounds; x++)
	{
		for (int y = -bounds; y < bounds; y++)
		{
			const v2 p = v2_mul_s({(float)x, (float)y}, 16);
			Circle c = {p, unit_size};
			entities.emplace_back(c);
		}
	}

	v2 n = {};

	AabbGrid<Circle> grid;

	auto t1 = std::chrono::high_resolution_clock::now();
	int collision_count = 0;

	grid.clear();

	for (auto &a : entities)
	{
		Aabb aabb = {a.p - V2(a.r, a.r), a.p + V2(a.r, a.r)};
		grid.insert(aabb, &a);
	}

	for (auto &e : entities)
	{
		Aabb aabb = {e.p - V2(e.r, e.r), e.p + V2(e.r, e.r)};
		grid.query(
			aabb,
			[&](Circle *a, Circle *b) {
				v2 n = {};
				if (circle_to_circle(*a, *b, &n))
				{
					a->p = v2_sub(a->p, n);
					b->p = v2_add(b->p, n);

					collision_count++;
				}

				return true;
			},
			&e
		);
	}

	auto t2 = std::chrono::high_resolution_clock::now();
	double frame_time = std::chrono::duration<double>(t2 - t1).count();
	std::cout << "frame_time = " << frame_time << std::endl;
	std::cout << "fps = " << 1.f / frame_time << std::endl;
	std::cout << "size = " << entities.size() << std::endl;
	std::cout << "collisions = " << collision_count << std::endl;
}

int main()
{
	regular_c_test();
	std::cout << std::endl;
	aabb_grid_test();
	return 0;
}
