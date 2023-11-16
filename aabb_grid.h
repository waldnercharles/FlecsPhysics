#pragma once
#include <functional>
#include <unordered_map>
#include <unordered_set>

template <class R, class... Args>
using Func [[maybe_unused]] = std::function<R(Args...)>;

template <typename T = float>
struct v2
{
	T x, y;

	inline v2 operator+(const v2 other) const
	{
		return {x + other.x, y + other.y};
	}

	inline v2 operator-(const v2 other) const
	{
		return {x - other.y, y - other.y};
	}

	template <typename S>
	inline v2<T> operator/(const S scalar) const
	{
		return {x / scalar, y / scalar};
	}

	template <typename S>
	explicit operator v2<S>() const
	{
		return {(S)x, (S)y};
	}
};

struct Aabb
{
	v2<float> min;
	v2<float> max;
};

using AabbCell = std::pair<int, int>;

struct AabbCellHash
{
	static_assert(sizeof(int) * 2 == sizeof(size_t));

	size_t operator()(const AabbCell &cell) const noexcept
	{
		return size_t(cell.first) << 32 | cell.second;
	}
};

template <typename T>
struct AabbGridNode
{
	int id;
	T *udata;

	AabbGridNode(int id, T *udata) : id(id), udata(udata)
	{
	}
};

template <typename T>
struct AabbGrid
{
	int cell_size = 16;

	std::unordered_map<AabbCell, std::vector<AabbGridNode<T>>, AabbCellHash>
		cells;

	int next_id = 0;

	void clear()
	{
		for (auto pair : cells)
		{
			pair.second.clear();
		}

		next_id = 0;
	}

	void insert(Aabb aabb, T *udata)
	{
		int id = ++next_id;
		for_each_cell(aabb, [=](AabbCell cell) {
			cells[cell].emplace_back(AabbGridNode<T>(id, udata));
		});
	}

	void query(Aabb aabb, Func<bool, T *, T *> fn, T *a_udata)
	{
		query_cache.clear();

		for_each_cell(aabb, [&](AabbCell cell) {
			for (AabbGridNode<T> node : cells[cell])
			{
				if (std::find(
						query_cache.begin(),
						query_cache.end(),
						node.id
					) != query_cache.end())
				{
					return;
				}
				query_cache.emplace_back(node.id);

				if (!fn(a_udata, node.udata))
				{
					return;
				}
			}
		});
	}

private:
	std::vector<int> query_cache;
	void for_each_cell(Aabb aabb, Func<void, AabbCell> fn)
	{
		auto min = (v2<int>)(aabb.min / cell_size);
		auto max = (v2<int>)(aabb.min / cell_size);

		for (int x = min.x; x <= max.x; x++)
		{
			for (int y = min.y; y <= min.y; y++)
			{
				fn({x, y});
			}
		}
	}
};
