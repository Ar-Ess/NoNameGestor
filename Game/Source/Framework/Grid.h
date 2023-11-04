#pragma once
#include "Array.h"
#include "Flag.h"
#include "Point.h"

template <class T>
class Grid
{
    struct Row
    {
        Row(unsigned int width)
        {
            unsigned int flagAmount = (unsigned int)ceil(width / 8.0f);
            for (unsigned int i = 0; i < flagAmount; ++i) full.Add(new Flag());
        }
        void SetValue(T value, unsigned int x)
        {
            SetFullFlag(x, true);
            row.Assign(value, x);
        }
        void SetFullFlag(unsigned int x, bool state)
        {
            unsigned int setIndex = x / Flag::Capacity();
            unsigned int flagIndex = x - Flag::Capacity() * setIndex;
            full.At(setIndex)->Set(flagIndex, state);
        }
        bool Empty(unsigned int x)
        {
            unsigned int setIndex = x / Flag::Capacity();
            unsigned int flagIndex = x - Flag::Capacity() * setIndex;
            return !full.At(setIndex)->Get(flagIndex);
        }
        bool Empty()
        {
            return size == 0;
        }
        T GetNonEmptyValue(unsigned int x, unsigned int width)
        {
            unsigned int fullCells = 0;
            for (unsigned int i = 0; i < width; ++i)
            {
                if (Empty(i)) continue;

                ++fullCells;
                if (x == fullCells - 1) return row.At(i);
            }

            return T();
        }
        void Clear()
        {
            size = 0;
            for (unsigned int i = 0; i < full.Size(); ++i) full[i]->Clear();
        }
        void CleanUp()
        {
            full.Clear();
            row.Clear();
        }

        Array<T> row;
        Array<Flag*> full;
        unsigned int size = 0;
    };

public:

    Grid(unsigned int width, unsigned int height)
    {
        this->width = width;
        this->height = height;
        this->grid = new Array<Row*>();
        for (unsigned int y = 0; y < height; ++y)
        {
            grid->Add(new Row(width));
            for (unsigned int x = 0; x < width; ++x) grid->At(y)->row.Add(T());
        }
    }

    Grid(Point size)
    {
        this->width = size.x;
        this->height = size.y;
        this->grid = new Array<Row*>();
        for (unsigned int y = 0; y < height; ++y)
        {
            grid->Add(new Row(width));
            for (unsigned int x = 0; x < width; ++x) grid->At(y)->row.Add(T());
        }
    }

    bool Set(T value, int x, int y)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;

        Row* row = GetRow(y);
        row->size++;

        row->SetValue(value, x);
        size++;
    }
    
    bool Set(T value, Point coords)
    {
        if (coords.x < 0 || coords.x >= width || coords.y < 0 || coords.y >= height) return false;

        Row* row = GetRow(coords.y);
        row->size++;

        row->SetValue(value, coords.x);
        size++;
    }

    bool Set(T value, int index)
    {
        if (index >= width * height || index < 0) return false;

        Point coords = FromIndexToCoords(index);

        Row* row = GetRow(coords.y);
        row->size++;

        row->SetValue(value, coords.x);
        size++;
    }

    T At(int x, int y) const
    {
        if (Empty() || x < 0 || x >= width || y < 0 || y >= height) return 0;

        return GetRow(y)->row.At(x);
    }

    T At(Point coords) const
    {
        if (Empty() || coords.x < 0 || coords.x >= width || coords.y < 0 || coords.y >= height) return 0;

        return GetRow(coords.y)->row.At(coords.x);
    }

    T At(int index) const
    {
        if (Empty() || index >= width * height || index < 0) return 0;

        Point coords = FromIndexToCoords(index);

        return GetRow(coords.y)->row.At(coords.x);
    }

    // Returns the amount of not empty nodes
    unsigned int Size() const
    {
        return size;
    }

    // Returns the total amount of nodes
    unsigned int SizeMax() const
    {
        return width * height;
    }

    unsigned int Width() const
    {
        return width;
    }

    unsigned int Heigth() const
    {
        return height;
    }

    // Returns the amount of not empty nodes in a row
    unsigned int SizeRow (int row) const
    {
        if (row < 0 || row >= width) return 0;

        return GetRow(row)->size;
    }

    bool Empty() const
    {
        return (size <= 0);
    }

    bool Empty(int x, int y) const
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;

        return GetRow(y)->Empty(x);
    }

    bool Empty(Point coords) const
    {
        if (coords.x < 0 || coords.x >= width || coords.y < 0 || coords.y >= height) return false;

        return GetRow(coords.y)->Empty(coords.x);
    }

    // Returns the not empty cells sequentially
    // Optimized to iterate
    T operator[](int index)
    {
        if (index < 0 || index >= size || Empty()) return 0;

        const unsigned int offset = index / width;

        for (unsigned int i = 0; i < height; ++i)
        {
            Row* row = GetRow(i);
            if (i >= offset && int(index) - int(row->size) < 0) return row->GetNonEmptyValue(index, width);
            index -= row->size;
        }

        return T();
    }

    Point Find(T value) const
    {
        Point ret = {-1, -1};
        if (Empty()) return ret;

        for (unsigned int i = 0; i < height; ++i)
        {
            Row* row = GetRow(i);
            if (row->Empty()) continue;
            int index = row->row.Find(value);
            if (index == -1) continue;
            ret = Point((float)index, (float)i);
            break;
        }

        return ret;
    }

    // Empty all cells
    bool Clear()
    {
        if (Empty()) return true;

        for (unsigned int i = 0; i < height; ++i) GetRow(i)->Clear();
        size = 0;

        return true;
    }

    // Clean the structure. Return to a 0 by 0 grid
    bool CleanUp()
    {
        for (unsigned int i = 0; i < height: ++i)
        {
            GetRow(i)->CleanUp();
            size = 0;
        }
        grid->Clear();
        delete grid;
        grid = nullptr;

        this->width = 0;
        this->height = 0;
        this->size = 0;

        return true;
    }

    bool Erase(int x, int y)
    {
        if (x < 0 || x >= width || y < 0 || y >= height || Empty()) return false;

        Row* row = GetRow(y);
        if (row->size <= 0) return false;

        row->SetFullFlag(x, false);
        
        row->size--;
        size--;

        return true;
    }
    
    bool Erase(Point coords)
    {
        if (coords.x < 0 || coords.x >= width || coords.y < 0 || coords.y >= height || Empty()) return false;

        Row* row = GetRow(coords.y);
        if (row->size <= 0) return false;

        row->SetFullFlag(coords.x, false);

        row->size--;
        size--;

        return true;
    }

    bool Erase(int index)
    {
        if (Empty() || index < 0 || index >= width * height) return false;

        Point coords = FromIndexToCoords(index);
        Row* row = GetRow(coords.y);
        if (row->size <= 0) return false;

        row->SetFullFlag(coords.x, false);
        
        row->size--;
        size--;

        return true;
    }

private:

    Row* GetRow(unsigned int y) const
    {
        return grid->At(y);
    }

    Point FromIndexToCoords(unsigned int index)
    {
        unsigned int y = index / width;
        unsigned int x = index - width * y;

        return {x, y};
    }

private:

    Array<Row*>* grid = nullptr;

    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int size = 0;

};

template <class T>
class Grid<T*>
{
    struct Row
    {
        void SetValue(T* value, unsigned int x)
        {
            row.Assign(value, x);
        }
        bool Empty(unsigned int x)
        {
            return row.At(x) == nullptr;
        }
        bool Empty()
        {
            return size == 0;
        }
        T* GetNonEmptyValue(unsigned int x, unsigned int width)
        {
            unsigned int fullCells = 0;
            for (unsigned int i = 0; i < width; ++i)
            {
                if (Empty(i)) continue;

                ++fullCells;
                if (x == fullCells - 1) return row.At(i);
            }

            return nullptr;
        }
        void Clear()
        {
            size = 0;
            for (unsigned int i = 0; i < row->Size(); ++i)
            {
                T* value = row[i];
                delete value;
                value = nullptr;
            }
        }
        void CleanUp()
        {
            row.Clear();
        }

        Array<T*> row;
        unsigned int size = 0;
    };

public:

    Grid(unsigned int width, unsigned int height)
    {
        this->width = width;
        this->height = height;
        this->grid = new Array<Row*>();
        for (unsigned int y = 0; y < height; ++y)
        {
            grid->Add(new Row());
            for (unsigned int x = 0; x < width; ++x) grid->At(y)->row.Add(nullptr);
        }
    }
    
    Grid(Point size)
    {
        this->width = size.x;
        this->height = size.y;
        this->grid = new Array<Row*>();
        for (unsigned int y = 0; y < height; ++y)
        {
            grid->Add(new Row());
            for (unsigned int x = 0; x < width; ++x) grid->At(y)->row.Add(nullptr);
        }
    }

    bool Set(T* value, unsigned int x, unsigned int y)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;

        Row* row = GetRow(y);
        row->size++;

        row->SetValue(value, x);
        size++;
    }

    bool Set(T* value, Point coords)
    {
        if (coords.x < 0 || coords.x >= width || coords.y < 0 || coords.y >= height) return false;

        Row* row = GetRow(coords.y);
        row->size++;

        row->SetValue(value, coords.x);
        size++;
    }

    bool Set(T* value, int index)
    {
        if (index >= width * height || index < 0) return false;

        Point coords = FromIndexToCoords(index);

        Row* row = GetRow(coords.y);
        row->size++;

        row->SetValue(value, coords.x);
        size++;
    }

    T* At(int x, int y) const
    {
        if (Empty() || x < 0 || x >= width || y < 0 || y >= height) return nullptr;

        return GetRow(y)->row.At(x);
    }

    T* At(Point coords) const
    {
        if (Empty() || coords.x < 0 || coords.x >= width || coords.y < 0 || coords.y >= height) return nullptr;

        return GetRow(coords.y)->row.At(coords.x);
    }

    T* At(int index) const
    {
        if (Empty() || index >= width * height || index < 0) return nullptr;

        Point coords = FromIndexToCoords(index);

        return GetRow(coords.y)->row.At(coords.x);
    }

    // Returns the amount of not empty nodes
    unsigned int Size() const
    {
        return size;
    }

    // Returns the total amount of nodes
    unsigned int SizeMax() const
    {
        return width * height;
    }

    unsigned int Width() const
    {
        return width;
    }

    unsigned int Heigth() const
    {
        return height;
    }

    // Returns the amount of not empty nodes in a row
    unsigned int SizeRow(int row) const
    {
        if (row < 0 || row >= width) return 0;

        return GetRow(row)->size;
    }

    bool Empty() const
    {
        return (size <= 0);
    }

    bool Empty(int x, int y) const
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;

        return GetRow(y)->Empty(x);
    }

    bool Empty(Point coords) const
    {
        if (coords.x < 0 || coords.x >= width || coords.y < 0 || coords.y >= height) return false;

        return GetRow(coords.y)->Empty(coords.x);
    }

    // Returns the not empty cells sequentially
    // Optimized to iterate
    T* operator[](unsigned int index)
    {
        if (index < 0 || index >= size || Empty()) return 0;
        const unsigned int offset = index / width;

        for (unsigned int i = 0; i < height; ++i)
        {
            Row* row = GetRow(i);
            if (i >= offset && int(index) - int(row->size) < 0) return row->GetNonEmptyValue(index, width);
            index -= row->size;
        }

        return nullptr;
    }

    Point Find(T* value) const
    {
        Point ret = { -1, -1 };
        if (Empty()) return ret;

        for (unsigned int i = 0; i < height; ++i)
        {
            Row* row = GetRow(i);
            if (row->Empty()) continue;
            int index = row->row.Find(value);
            if (index == -1) continue;
            ret = Point((float)index, (float)i );
            break;
        }

        return ret;
    }

    // Empty all cells
    bool Clear()
    {
        if (Empty()) return true;

        for (unsigned int i = 0; i < height; ++i) GetRow(i)->Clear();
        size = 0;

        return true;
    }

    // Clean the structure. Return to a 0 by 0 grid
    bool CleanUp()
    {
        for (unsigned int i = 0; i < height: ++i)
        {
            GetRow(i)->CleanUp();
        }
        grid->Clear();
        delete grid;
        grid = nullptr;

        this->width = 0;
        this->height = 0;
        this->size = 0;

        return true;
    }

    bool Erase(int x, int y)
    {
        if (x < 0 || x >= width || y < 0 || y >= height || Empty()) return false;

        Row* row = GetRow(y);
        if (row->size <= 0) return false;

        // No "row.Erase()" it would erase the node too
        T* value = row->row[x];
        delete value;
        value = nullptr;

        row->size--;
        size--;

        return true;
    }

    bool Erase(Point coords)
    {
        if (coords.x < 0 || coords.x >= width || coords.y < 0 || coords.y >= height || Empty()) return false;

        Row* row = GetRow(y);
        if (row->size <= 0) return false;

        // No "row.Erase()" it would erase the node too
        T* value = row->row[x];
        delete value;
        value = nullptr;

        row->size--;
        size--;

        return true;
    }

    bool Erase(int index)
    {
        if (Empty() || index < 0 || index >= width * height) return false;

        Point coords = FromIndexToCoords(index);
        Row* row = GetRow(coords.y);
        if (row->size <= 0) return false;

        // No "row.Erase()" it would erase the node too
        T* value = row->row[x];
        delete value;
        value = nullptr;

        row->size--;
        size--;

        return true;
    }

private:
    
    Row* GetRow(unsigned int y) const
    {
        return grid->At(y);
    }

    Point FromIndexToCoords(unsigned int index)
    {
        unsigned int y = index / width;
        unsigned int x = index - width * y;

        return { x, y };
    }

private:

    Array<Row*>* grid = nullptr;

    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int size = 0;

};
