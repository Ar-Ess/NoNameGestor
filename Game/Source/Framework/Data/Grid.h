#pragma once

#include "Framework/Utils/Nullable.h"
#include "Framework/Data/Array.h"
#include "Framework/Shapes/Point.h"

template<class T, bool Owns = false>
class Grid
{
	template<class, bool>
	friend class Grid;

	static_assert(!Owns || std::is_pointer_v<T>, "Grid<T, true> only allowed when T is a pointer mode.");

    typedef Nullable<T, Owns> Cell;

public:

    /// Creates an empty grid with the specified dimensions.
    /// Cells are initialized without values.
	Grid(int width, int height) :
		grid(nullptr), width(0), height(0)
	{
        ConstructInternal(width, height, nullptr);
	}

    /// Creates a grid with the specified dimensions.
    /// Every cell is initialized with the provided value.
    Grid(int width, int height, const T& initial) :
        grid(nullptr), width(0), height(0)
    {
        static_assert(std::is_copy_constructible_v<T>, "Grid::Grid(int, int, const T&) requires T to be copy constructible.");
        ConstructInternal(width, height, initial);
    }

    /// Creates an empty grid using a Point as dimensions.
    /// Point.x = width, Point.y = height.
    Grid(const Point& size) :
        grid(nullptr), width(0), height(0)
    {
        ConstructInternal(size.x, size.y, nullptr);
    }

    /// Creates a grid using a Point as dimensions.
    /// Every cell is initialized with the provided value.
    Grid(const Point& size, const T& initial) :
        grid(nullptr), width(0), height(0)
    {
        static_assert(std::is_copy_constructible_v<T>, "Grid::Grid(int, int, const T&) requires T to be copy constructible.");
        ConstructInternal(size.x, size.y, initial);
    }

    /// Transfers ownership of another grid's data.
    Grid(Grid&& other) noexcept
    {
        grid = other.grid;
        width = other.width;
        height = other.height;
        size = other.size;

        other.grid = nullptr;
        other.width = 0;
        other.height = 0;
        other.size = 0;
    }

    Grid(const Grid&) = delete;

	~Grid()
	{
		Clear();
	}

    /// Creates a deep copy of this grid into a non-owning destination grid.
    void CopyTo(Grid<T>& out) const
    {
        static_assert(std::is_copy_constructible_v<T>, "Grid::CopyTo() requires T to be copy constructible.");

        if (reinterpret_cast<const void*>(&out) ==
            reinterpret_cast<const void*>(this))
            return;

        out.Clear();

        if (IsNull())
            return;

        out.grid = new Nullable<T>[width * height];

        for (int i = 0; i < width * height; ++i)
            out.grid[i] = grid[i];

        out.width = width;
        out.height = height;
        out.size = size;
    }

    void CopyTo(Grid<T, true>&) const = delete;

    /// Removes all cells and resets the grid to a null state.
	void Clear()
	{
		delete[] grid;
		grid = nullptr;
        width = 0;
        height = 0;
        size = 0;
	}

    /// Returns true if the internal storage is null.
	bool IsNull() const
	{
		return grid == nullptr;
	}

    /// Returns true if the grid is null or contains no values.
    bool IsNullOrEmpty() const
    {
        return IsNull() || size == 0;
    }

    /// Returns true if every cell contains a value.
    bool IsFull() const
    {
        return size == (width * height);
    }

    /// Returns the amount of occupied cells.
    int Size() const
    {
        return size;
    }

    /// Returns the amount of occupied cells in a row.
    int RowSize(int row) const
    {
        Debug::Assert(row >= 0 && row < height, "Grid::RowSize(int) inputted row is out of bounds!");

        if (IsFull())
            return width;

        if (IsNullOrEmpty())
            return 0;

        int count = 0;
        for (int i = 0; i < width; ++i)
            count += GetCell(i, row).HasValue() ? 1 : 0;
        return count;
    }

    /// Returns the amount of occupied cells in a column.
    int ColumnSize(int column) const
    {
        Debug::Assert(column >= 0 && column < width, "Grid::ColumnSize(int) inputted column is out of bounds!");

        if (IsFull())
            return height;

        if (IsNullOrEmpty())
            return 0;

        int count = 0;
        for (int i = 0; i < height; ++i)
            count += GetCell(column, i).HasValue() ? 1 : 0;
        return count;
    }

    /// Returns true if the specified row contains no values.
    bool IsRowEmpty(int row) const
    {
        return RowSize(row) == 0;
    }

    /// Returns true if every cell in the specified row contains a value.
    bool IsRowFull(int row) const
    {
        return RowSize(row) == width;
    }

    /// Returns true if the specified column contains no values.
    bool IsColumnEmpty(int column) const
    {
        return ColumnSize(column) == 0;
    }

    /// Returns true if every cell in the specified column contains a value.
    bool IsColumnFull(int column) const
    {
        return ColumnSize(column) == height;
    }

    /// Returns true if every cell in the specified column contains a value.
    int Area() const
    {
        return width * height;
    }

    /// Returns the grid width.
    int Width() const
    {
        return width;
    }

    /// Returns the grid height.
    int Height() const
    {
        return height;
    }

    /// Changes the dimensions of the grid.
    /// New cells are initialized empty.
    void Resize(int width, int height)
    {
        ResizeInternal(width, height, nullptr);
    }

    /// Changes the dimensions of the grid.
    /// New cells are initialized with the provided value.
    void Resize(int width, int height, const T& initial)
    {
        static_assert(std::is_copy_constructible_v<T>, "Grid::Resize(int, int, const T&) requires T to be copy constructible.");
        ResizeInternal(width, height, initial);
    }

    /// Changes the dimensions of the grid.
    /// New cells are initialized empty.
    void Resize(const Point& size)
    {
        ResizeInternal(size.x, size.y, nullptr);
    }

    /// Changes the dimensions of the grid.
    /// New cells are initialized with the provided value.
    void Resize(const Point& size, const T& initial)
    {
        static_assert(std::is_copy_constructible_v<T>, "Grid::Resize(const Point& size, const T&) requires T to be copy constructible.");
        ResizeInternal(size.x, size.y, initial);
    }

    /// Assigns a value to the specified cell.
    void Assign(const T& value, int x, int y)
    {
        static_assert(std::is_copy_constructible_v<T>, "Grid::Assign(const T&, int, int) requires T to be copy constructible.");
        Debug::Assert(x >= 0 && y >= 0 && x < width && y < height, "Grid::Assign(const T&, int, int) inputted coords are out of bounds!");
        Cell& cell = GetCell(x, y);

        if constexpr (std::is_pointer_v<T>)
        {
            if (value == nullptr)
            {
                if (cell.HasValue())
                {
                    cell.Clear();
                    --size;
                }

                return;
            }
        }

        if (!cell.HasValue())
            ++size;

        cell = value;
    }

    /// Moves a value into the specified cell.
    void Assign(T&& value, int x, int y)
    {
        static_assert(std::is_move_constructible_v<T>, "Grid::Assign(const T&, int, int) requires T to be move constructible.");
        Debug::Assert(x >= 0 && y >= 0 && x < width && y < height, "Grid::Assign(const T&, int, int) inputted coords are out of bounds!");
        Cell& cell = GetCell(x, y);

        if constexpr (std::is_pointer_v<T>)
        {
            if (value == nullptr)
            {
                if (cell.HasValue())
                {
                    cell.Clear();
                    --size;
                }

                return;
            }
        }

        if (!cell.HasValue())
            ++size;

        cell = std::move(value);
    }

    /// Assigns a value using Point coordinates.
    void Assign(const T& value, const Point& size)
    {
        static_assert(std::is_copy_constructible_v<T>, "Grid::Assign(const T&, const Point& size) requires T to be copy constructible.");
        Debug::Assert(size.x >= 0 && size.y >= 0 && size.x < width && size.y < height, "Grid::Assign(const T&, const Point& size) inputted coords are out of bounds!");
        Cell& cell = GetCell(size.x, size.y);

        if constexpr (std::is_pointer_v<T>)
        {
            if (value == nullptr)
            {
                if (cell.HasValue())
                {
                    cell.Clear();
                    --size;
                }

                return;
            }
        }

        if (!cell.HasValue())
            ++size;

        cell = value;
    }

    /// Moves a value using Point coordinates.
    void Assign(T&& value, const Point& size)
    {
        static_assert(std::is_move_constructible_v<T>, "Grid::Assign(const T&, const Point& size) requires T to be move constructible.");
        Debug::Assert(size.x >= 0 && size.y >= 0 && size.x < width && size.y < height, "Grid::Assign(const T&, const Point& size) inputted coords are out of bounds!");
        Cell& cell = GetCell(size.x, size.y);

        if constexpr (std::is_pointer_v<T>)
        {
            if (value == nullptr)
            {
                if (cell.HasValue())
                {
                    cell.Clear();
                    --size;
                }

                return;
            }
        }

        if (!cell.HasValue())
            ++size;

        cell = std::move(value);
    }

    /// Removes the value stored at the specified coordinates.
    bool Erase(int x, int y)
    {
        Debug::Assert(x >= 0 && y >= 0 && x < width && y < height, "Grid::Erase(int, int) inputted coords are out of bounds!");
        
        Cell& cell = GetCell(x, y);
        if (!cell.HasValue())
            return false;
        
        cell.Clear();
        size--;
        return true;
    }

    /// Removes the value stored at the specified coordinates.
    bool Erase(const Point& size)
    {
        Debug::Assert(size.x >= 0 && size.y >= 0 && size.x < width && size.y < height, "Grid::Erase(const Point&) inputted coords are out of bounds!");

        Cell& cell = GetCell(size.x, size.y);
        if (!cell.HasValue())
            return false;

        cell.Clear();
        size--;
        return true;
    }

    /// Removes all values located at the provided coordinates.
    bool Erase(const Array<Point>& points)
    {
        bool allGood = true;

        points.Iterate([&](const Point& p)
            {
                if (p.x < 0 || p.y < 0 || p.x >= width || p.y >= height)
                {
                    allGood = false;
                    return;
                }

                Erase(p.x, p.y);
            });

        return allGood;
    }

    /// Returns the value stored at the specified coordinates.
    /// Fails if coordinates are invalid or empty.
    T& At(int x, int y)
    {
        Debug::Assert(x >= 0 && y >= 0 && x < width && y < height, "Grid::At(int, int) inputted coords are out of bounds!");
        Cell& cell = GetCell(x, y);

        Debug::Assert(cell.HasValue(), "Grid::At(int, int) inputted coords lead to an empty value!");
        return cell.Value();
    }

    /// Returns the value stored at the specified coordinates.
    /// Fails if coordinates are invalid or empty.
    T& At(const Point& size)
    {
        Debug::Assert(size.x >= 0 && size.y >= 0 && size.x < width && size.y < height, "Grid::At(const Point&) inputted coords are out of bounds!");
        Cell& cell = GetCell(size.x, size.y);

        Debug::Assert(cell.HasValue(), "Grid::At(const Point&) inputted coords lead to an empty value!");
        return cell.Value();
    }

    /// Returns the value stored at the specified coordinates.
    /// Fails if coordinates are invalid or empty.
    const T& At(int x, int y) const
    {
        Debug::Assert(x >= 0 && y >= 0 && x < width && y < height, "Grid::At(int, int) inputted coords are out of bounds!");
        const Cell& cell = GetCell(x, y);

        Debug::Assert(cell.HasValue(), "Grid::At(int, int) inputted coords lead to an empty value!");
        return cell.Value();
    }

    /// Returns the value stored at the specified coordinates.
    /// Fails if coordinates are invalid or empty.
    const T& At(const Point& size) const
    {
        Debug::Assert(size.x >= 0 && size.y >= 0 && size.x < width && size.y < height, "Grid::At(const Point&) inputted coords are out of bounds!");
        Cell& cell = GetCell(size.x, size.y);

        Debug::Assert(cell.HasValue(), "Grid::At(const Point&) inputted coords lead to an empty value!");
        return cell.Value();
    }

    /// Attempts to retrieve the value at the specified coordinates.
    bool TryAt(int x, int y, OutParameter<T> out)
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return false;

        Cell& cell = GetCell(x, y);
        if constexpr (std::is_pointer_v<T>)
        {
            out = cell.ValueOr(nullptr);

            return cell.HasValue();
        }
        else
        {
            if (cell.HasValue())
                out = cell.Value();

            return cell.HasValue();
        }
    }

    /// Attempts to retrieve the value at the specified coordinates.
    bool TryAt(int x, int y, OutParameter<const T> out) const
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return false;

        const Cell& cell = GetCell(x, y);
        if constexpr (std::is_pointer_v<T>)
        {
            out = cell.ValueOr(nullptr);

            return cell.HasValue();
        }
        else
        {
            if (cell.HasValue())
                out = cell.Value();

            return cell.HasValue();
        }
    }

    /// Attempts to retrieve the value at the specified coordinates.
    bool TryAt(const Point& size, OutParameter<T> out)
    {
        if (size.x < 0 || size.y < 0 || size.x >= width || size.y >= height)
            return false;

        Cell& cell = GetCell(size.x, size.y);
        if constexpr (std::is_pointer_v<T>)
        {
            out = cell.ValueOr(nullptr);

            return cell.HasValue();
        }
        else
        {
            if (cell.HasValue())
                out = cell.Value();

            return cell.HasValue();
        }
    }

    /// Attempts to retrieve the value at the specified coordinates.
    bool TryAt(const Point& size, OutParameter<const T> out) const
    {
        if (size.x < 0 || size.y < 0 || size.x >= width || size.y >= height)
            return false;

        const Cell& cell = GetCell(size.x, size.y);
        if constexpr (std::is_pointer_v<T>)
        {
            out = cell.ValueOr(nullptr);

            return cell.HasValue();
        }
        else
        {
            if (cell.HasValue())
                out = cell.Value();

            return cell.HasValue();
        }
    }

    /// Returns true if a value exists at the specified coordinates.
    bool Exists(int x, int y) const
    {
        Debug::Assert(x >= 0 && y >= 0 && x < width && y < height, "Grid::Exists(int, int) inputted coords are out of bounds!");
        return GetCell(x, y).HasValue();
    }

    /// Returns true if a value exists at the specified coordinates.
    bool Exists(const Point& size) const
    {
        Debug::Assert(size.x >= 0 && size.y >= 0 && size.x < width && size.y < height, "Grid::Exists(const Point& size) inputted coords are out of bounds!");
        return GetCell(size.x, size.y).HasValue();
    }

    /// Iterates every occupied cell in reading order.
    /// Supports:
    /// (T&)
    /// (T&, Point)
    /// (T&, Point, int)
    /// and bool-returning versions to stop iteration.
    template<typename Callable>
    void Iterate(Callable&& func)
    {
        IterateInternal(*this, std::forward<Callable>(func));
    }

    /// Iterates every occupied cell in reading order.
    /// Supports:
    /// (const T&)
    /// (const T&, Point)
    /// (const T&, Point, int)
    /// and bool-returning versions to stop iteration.
    template<typename Callable>
    void Iterate(Callable&& func) const
    {
        IterateInternal(*this, std::forward<Callable>(func));
    }

    /// Finds the first occurrence of a value.
    /// Returns Point(-1,-1) if not found.
    template<typename U>
        requires Comparable<T, U>
    Point Find(const U& value) const
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                if constexpr (std::is_pointer_v<T>)
                {
                    if (cell.Value() == nullptr || value == nullptr)
                    {
                        if (cell.Value() == value)
                            return Point(x, y);
                    }
                    else if (*cell.Value() == *value)
                    {
                        return Point(x, y);
                    }
                }
                else
                {
                    if (cell.Value() == value)
                        return Point(x, y);
                }
            }
        }

        return Point(-1, -1);
    }

    /// Finds the first value matching a predicate.
    /// Returns Point(-1,-1) if not found.
    template<typename Callable>
        requires Predicate<Callable, T>
    Point Find(Callable&& predicate) const
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                if (predicate(cell.Value()))
                    return Point(x, y);
            }
        }

        return Point(-1, -1);
    }

    /// Returns coordinates of all matching values.
    template<typename U>
        requires Comparable<T, U>
    Array<Point> FindAll(const U& value) const
    {
        Point* result = new Point[size];

        int count = 0;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                bool match = false;

                if constexpr (std::is_pointer_v<T>)
                {
                    if (cell.Value() == nullptr || value == nullptr)
                    {
                        match = (cell.Value() == value);
                    }
                    else
                    {
                        match = (*cell.Value() == *value);
                    }
                }
                else
                {
                    match = (cell.Value() == value);
                }

                if (match)
                    result[count++] = Point(x, y);
            }
        }

        Array<Point> ret(result, count);
        delete[] result;

        return ret;
    }

    /// Returns coordinates of all values matching a predicate.
    template<typename Callable>
        requires Predicate<Callable, T>
    Array<Point> FindAll(Callable&& predicate) const
    {
        Point* result = new Point[size];

        int count = 0;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                if (predicate(cell.Value()))
                    result[count++] = Point(x, y);
            }
        }

        Array<Point> ret(result, count);
        delete[] result;

        return ret;
    }

    /// Retrieves the first value matching a predicate.
    template<typename Callable>
        requires Predicate<Callable, T>
    bool Retrieve(Callable&& predicate, OutParameter<T> out)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                if (predicate(cell.Value()))
                {
                    if constexpr (std::is_pointer_v<T>)
                        out = cell.Value();
                    else
                        out = &cell.Value();

                    return true;
                }
            }
        }

        return false;
    }

    /// Retrieves the first value matching a predicate.
    template<typename Callable>
        requires Predicate<Callable, T>
    bool Retrieve(Callable&& predicate, OutParameter<const T> out) const
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                if (predicate(cell.Value()))
                {
                    if constexpr (std::is_pointer_v<T>)
                        out = cell.Value();
                    else
                        out = &cell.Value();

                    return true;
                }
            }
        }

        return false;
    }

    /// Retrieves all values matching a predicate.
    template<typename Callable>
        requires Predicate<Callable, T>
    bool RetrieveAll(Callable&& predicate, Array<OutParameterList<T>>& out)
    {
        if (IsNullOrEmpty())
        {
            out.Clear();
            return false;
        }

        OutParameterList<T>* result = new OutParameterList<T>[size];

        int count = 0;
        bool found = false;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                if (predicate(cell.Value()))
                {
                    if constexpr (std::is_pointer_v<T>)
                        result[count++] = cell.Value();
                    else
                        result[count++] = &cell.Value();

                    found = true;
                }
            }
        }

        out.Clear();
        out = Array<OutParameterList<T>>(result, count);

        delete[] result;

        return found;
    }

    /// Retrieves all values matching a predicate.
    template<typename Callable>
        requires Predicate<Callable, T>
    bool RetrieveAll(Callable&& predicate,
        Array<OutParameterList<const T>>& out) const
    {
        if (IsNullOrEmpty())
        {
            out.Clear();
            return false;
        }

        OutParameterList<const T>* result =
            new OutParameterList<const T>[size];

        int count = 0;
        bool found = false;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                if (predicate(cell.Value()))
                {
                    if constexpr (std::is_pointer_v<T>)
                        result[count++] = cell.Value();
                    else
                        result[count++] = &cell.Value();

                    found = true;
                }
            }
        }

        out.Clear();
        out = Array<OutParameterList<const T>>(result, count);

        delete[] result;

        return found;
    }

    /// Returns all existing values as a contiguous array.
    Array<T, false> ToArray() const
    {
        if (IsNullOrEmpty())
            return Array<T>();

        static_assert(std::is_copy_constructible_v<T>, "Grid::ToArray() requires T to be copy constructible.");

        static_assert(std::is_copy_assignable_v<T>,"Grid::ToArray() requires T to be copy assignable.");

        Array<T> result(size);

        int index = 0;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                result[index++] = cell.Value();
            }
        }

        return result;
    }

    /// Removes the first matching value.
    template<typename U>
        requires Comparable<T, U>
    bool Remove(const U& value)
    {
        Point p = Find(value);

        if (p.x == -1)
            return false;

        return Erase(p.x, p.y);
    }

    /// Removes the first value matching a predicate.
    template<typename Callable>
        requires Predicate<Callable, T>
    bool Remove(Callable&& predicate)
    {
        Point p = Find(std::forward<Callable>(predicate));

        if (p.x == -1)
            return false;

        return Erase(p.x, p.y);
    }

    /// Removes all matching values.
    template<typename U>
        requires Comparable<T, U>
    bool RemoveAll(const U& value)
    {
        bool removed = false;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                bool match = false;

                if constexpr (std::is_pointer_v<T>)
                {
                    if (cell.Value() == nullptr || value == nullptr)
                    {
                        match = (cell.Value() == value);
                    }
                    else
                    {
                        match = (*cell.Value() == *value);
                    }
                }
                else
                {
                    match = (cell.Value() == value);
                }

                if (match)
                {
                    cell.Clear();
                    --size;
                    removed = true;
                }
            }
        }

        return removed;
    }

    /// Removes all values matching a predicate.
    template<typename Callable>
        requires Predicate<Callable, T>
    bool RemoveAll(Callable&& predicate)
    {
        bool removed = false;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                Cell& cell = GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                if (predicate(cell.Value()))
                {
                    cell.Clear();
                    --size;
                    removed = true;
                }
            }
        }

        return removed;
    }

public:

    /// Direct cell access without validation.
    T& operator()(int x, int y)
    {
        return GetCell(x, y).Value();
    }

    /// Direct cell access without validation.
    const T& operator()(int x, int y) const
    {
        return GetCell(x, y).Value();
    }

    /// Direct cell access without validation.
    T& operator()(const Point& size)
    {
        return GetCell(size.x, size.y).Value();
    }

    /// Direct cell access without validation.
    const T& operator()(const Point& size) const
    {
        return GetCell(size.x, size.y).Value();
    }

    Grid& operator=(Grid&& other) noexcept
    {
        if (this == &other)
            return *this;

        Clear();

        grid = other.grid;
        width = other.width;
        height = other.height;
        size = other.size;

        other.grid = nullptr;
        other.width = 0;
        other.height = 0;
        other.size = 0;

        return *this;
    }

    Grid& operator=(const Grid&) = delete;

private:

	int Index(int x, int y) const
	{
		return y * width + x;
	}

    Cell& GetCell(int x, int y)
    {
        return grid[Index(x, y)];
    }

    const Cell& GetCell(int x, int y) const
    {
        return grid[Index(x, y)];
    }

    void ConstructInternal(int w, int h, Nullable<const T&> initial)
    {
        Debug::Assert(w >= 0 && h >= 0, "Grid constructor can't initialize with a width or height smaller than 0");

        if (w == 0 || h == 0)
            return Clear();

        if (initial.HasValue())
        {
            size = w * h;
            grid = new Cell[w * h](initial);
        }
        else
        {
            size = 0;
            grid = new Cell[w * h];
        }

        width = w;
        height = h;
    }

	void ResizeInternal(int w, int h, Nullable<const T&> initial)
	{
        if (w == width && h == height)
            return;

        int oldWidth = width;
        int oldHeight = height;
        Cell* old = grid;
        ConstructInternal(w, h, initial);

        if (grid == nullptr)
            return;

        int iterW = (w < oldWidth) ? w : oldWidth;
        int iterH = (h < oldHeight) ? h : oldHeight;
        int newSize = 0;

		for (int x = 0; x < iterW; ++x)
		{
			for (int y = 0; y < iterH; ++y)
			{
                const Cell& cell = old[y * oldWidth + x];
				grid[y * w + x] = cell;
                if (cell.HasValue())
                    newSize++;
			}
		}

        size = initial.HasValue() ? w * h : newSize;
		width = w;
		height = h;
		delete[] old;
	}

    template<typename Self, typename Callable>
    void IterateInternal(Self& self, Callable&& func) const
    {
        if (self.IsNullOrEmpty())
            return;

        using ValueType = std::conditional_t<
            std::is_const_v<Self>,
            const T&,
            T&
        >;

        for (int y = 0; y < self.height; ++y)
        {
            for (int x = 0; x < self.width; ++x)
            {
                auto& cell = self.GetCell(x, y);

                if (!cell.HasValue())
                    continue;

                ValueType value = cell.Value();

                Point coords(x, y);

                bool shouldContinue = true;

                if constexpr (requires { func(value, coords, self.size); })
                {
                    if constexpr (std::is_convertible_v<
                        decltype(func(value, coords, self.size)),
                        bool>)
                    {
                        shouldContinue = func(value, coords, self.size);
                    }
                    else
                    {
                        func(value, coords, self.size);
                    }
                }
                else if constexpr (requires { func(value, coords); })
                {
                    if constexpr (std::is_convertible_v<
                        decltype(func(value, coords)),
                        bool>)
                    {
                        shouldContinue = func(value, coords);
                    }
                    else
                    {
                        func(value, coords);
                    }
                }
                else if constexpr (requires { func(value); })
                {
                    if constexpr (std::is_convertible_v<
                        decltype(func(value)),
                        bool>)
                    {
                        shouldContinue = func(value);
                    }
                    else
                    {
                        func(value);
                    }
                }
                else
                {
                    static_assert(sizeof(Callable) == 0, "Invalid callable for Grid::Iterate()");
                }

                if (!shouldContinue)
                    return;
            }
        }
    }

private:

    Cell* grid = nullptr;
	int width = 0;
	int height = 0;
    int size = 0;
};
