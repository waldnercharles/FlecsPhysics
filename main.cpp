#include <cmath>
#include <flecs.h>
#include <iostream>

struct Cell
{
	int x, y;
};

struct v2
{
	float x, y;
};

v2 v2_add(v2 a, v2 b)
{
	return {a.x + b.x, a.y + b.y};
}

v2 v2_sub(v2 a, v2 b)
{
	return {a.x - b.x, a.y - b.y};
}

float v2_dot(v2 a, v2 b)
{
	return a.x * b.x + a.y * b.y;
}

v2 v2_mul_s(v2 a, float b)
{
	a.x *= b;
	a.y *= b;
	return a;
}

struct Circle
{
	v2 p;
	float r;
};

bool circle_to_circle(Circle a, Circle b, v2 *out)
{
	v2 d = v2_sub(b.p, a.p);
	float d2 = v2_dot(d, d);
	float r = a.r + b.r;

	if (d2 < r * r)
	{
		float l = sqrtf(d2);
		v2 n = l != 0 ? v2_mul_s(d, 1.f / l) : (v2) {0.f, 1.f};
		n = v2_mul_s(n, r - l);
		*out = n;

		return true;
	}

	return false;
}

int main()
{
	flecs::world world;
	flecs::query<Circle> q = world.query<Circle>();

	const float unit_size = 12.f;

	// Note: With this implementation, this needs to be at least the largest obj size
	const float cell_size = 16.f;

	const int num_entities = 5000;
	int bounds = (int)ceil(sqrt(num_entities) / 2.f);

	for (int x = -bounds; x < bounds; x++)
	{
		for (int y = -bounds; y < bounds; y++)
		{
			const v2 val = v2_mul_s({(float)x, (float)y}, unit_size * 0.8f);
			world.entity().set<Circle>({val, unit_size});
		}
	}

	int32_t count = 0, test_count = 0, collision_count = 0;

	ecs_time_t t = {};
	ecs_time_measure(&t);


	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it))
	{
		Circle *a_circle = ecs_field(&it, Circle, 1);

		for (int i = 0; i < it.count; i++)
		{
			Cell a_cell = {
				(int)(a_circle[i].p.x / cell_size),
				(int)(a_circle[i].p.y / cell_size)};

			// Increment count before our inner loop so that we skip a == a check
			count++;

			ecs_iter_t nested_iter = ecs_query_iter(world, q);
			ecs_iter_t pit = ecs_page_iter(&nested_iter, count, 0);

			while (ecs_page_next(&pit))
			{
				Circle *b_circle = ecs_field(&pit, Circle, 1);

				for (int j = 0; j < pit.count; j++)
				{
					Cell b_cell = {
						(int)(b_circle[j].p.x / cell_size),
						(int)(b_circle[j].p.y / cell_size)};

					test_count++;

					if (abs(a_cell.x - b_cell.x) <= 1 &&
						abs(a_cell.y - b_cell.y) <= 1)
					{
						v2 n = {};
						if (circle_to_circle(a_circle[i], b_circle[j], &n))
						{
							a_circle[i].p = v2_sub(a_circle[i].p, n);
							b_circle[j].p = v2_add(b_circle[j].p, n);

							collision_count++;
						}
					}
				}
			}
		}
	}

	double frame_time = ecs_time_measure(&t);
	printf("frame_time = %f\n", frame_time);
	printf("fps = %f\n", 1.f / frame_time);
	printf("count = %i\n", count);
	printf("test_count = %i\n", test_count);
	printf("collision_count = %i\n", collision_count);

	return 0;
}
