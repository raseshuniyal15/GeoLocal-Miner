#ifndef GRID_ARRAY_H
#define GRID_ARRAY_H

#include <vector>
#include <stdexcept>
#include <cstddef>
#include <utility>

namespace GeoLocalMiner {

template <typename T>
class GridArray
{
private:
    size_t rowsCount = 0;
    size_t colsCount = 0;

    std::vector<T> data;

public:

    GridArray() = default;

    GridArray(size_t rows, size_t cols)
        : rowsCount(rows),
          colsCount(cols),
          data(rows * cols)
    {
    }

    GridArray(
        size_t rows,
        size_t cols,
        const T& value
    )
        : rowsCount(rows),
          colsCount(cols),
          data(rows * cols, value)
    {
    }


    GridArray(const GridArray&) = default;
    GridArray& operator=(const GridArray&) = default;

    GridArray(GridArray&&) noexcept = default;
    GridArray& operator=(GridArray&&) noexcept = default;

    ~GridArray() = default;


    void resize(size_t rows, size_t cols)
    {
        rowsCount = rows;
        colsCount = cols;
        data.resize(rows * cols);
    }


    void resize(
        size_t rows,
        size_t cols,
        const T& value
    )
    {
        rowsCount = rows;
        colsCount = cols;
        data.assign(rows * cols, value);
    }


    void clear()
    {
        rowsCount = 0;
        colsCount = 0;
        data.clear();
    }


    size_t rows() const
    {
        return rowsCount;
    }

    size_t cols() const
    {
        return colsCount;
    }

    size_t size() const
    {
        return data.size();
    }

    bool empty() const
    {
        return data.empty();
    }


    bool inBounds(size_t r, size_t c) const
    {
        return r < rowsCount && c < colsCount;
    }


    T& operator()(size_t r, size_t c)
    {
        return data[r * colsCount + c];
    }

    const T& operator()(size_t r, size_t c) const
    {
        return data[r * colsCount + c];
    }


    T& at(size_t r, size_t c)
    {
        if (!inBounds(r, c))
            throw std::out_of_range("Grid index out of bounds");

        return data[r * colsCount + c];
    }

    const T& at(size_t r, size_t c) const
    {
        if (!inBounds(r, c))
            throw std::out_of_range("Grid index out of bounds");

        return data[r * colsCount + c];
    }


    T& atIndex(size_t index)
    {
        if (index >= data.size())
            throw std::out_of_range("Grid index out of bounds");

        return data[index];
    }

    const T& atIndex(size_t index) const
    {
        if (index >= data.size())
            throw std::out_of_range("Grid index out of bounds");

        return data[index];
    }


    std::vector<T>& rawData()
    {
        return data;
    }

    const std::vector<T>& rawData() const
    {
        return data;
    }


    auto begin()
    {
        return data.begin();
    }

    auto end()
    {
        return data.end();
    }

    auto begin() const
    {
        return data.begin();
    }

    auto end() const
    {
        return data.end();
    }
};

}

#endif