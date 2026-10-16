#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <queue>
#include <cstddef>

namespace GeoLocalMiner {

struct SpatialEdge
{
    int id;
    double weight;

    SpatialEdge(int target, double w = 1.0)
        : id(target), weight(w)
    {
    }
};


class SpatialGraph
{
private:
    int vertices = 0;
    size_t edges = 0;

    std::vector<std::vector<SpatialEdge>> graph;

public:

    SpatialGraph() = default;

    explicit SpatialGraph(int n)
        : vertices(n),
          graph(n)
    {
    }


    void resize(int n)
    {
        vertices = n;
        edges = 0;
        graph.assign(n, {});
    }


    void clear()
    {
        vertices = 0;
        edges = 0;
        graph.clear();
    }


    int vertexCount() const
    {
        return vertices;
    }


    size_t edgeCount() const
    {
        return edges;
    }


    bool inBounds(int id) const
    {
        return id >= 0 && id < vertices;
    }


    void addEdge(
        int u,
        int v,
        double weight = 1.0,
        bool bothWays = true
    )
    {
        if (!inBounds(u) || !inBounds(v))
            return;

        graph[u].push_back(
            SpatialEdge(v, weight)
        );
        ++edges;

        if (bothWays && u != v)
        {
            graph[v].push_back(
                SpatialEdge(u, weight)
            );
            ++edges;
        }
    }


    const std::vector<SpatialEdge>& getNeighbors(
        int id
    ) const
    {
        static const std::vector<SpatialEdge> empty;

        if (!inBounds(id))
            return empty;

        return graph[id];
    }


    std::vector<int> getNeighborIds(int id) const
    {
        if (!inBounds(id))
            return {};

        std::vector<int> ids;
        ids.reserve(graph[id].size());

        for (const auto& edge : graph[id])
            ids.push_back(edge.id);

        return ids;
    }


    size_t getDegree(int id) const
    {
        if (!inBounds(id))
            return 0;

        return graph[id].size();
    }


    void buildGridTopology(
        size_t rows,
        size_t cols,
        bool diagonals = true
    )
    {
        resize(static_cast<int>(rows * cols));

        for (size_t r = 0; r < rows; ++r)
        {
            for (size_t c = 0; c < cols; ++c)
            {
                int id =
                    static_cast<int>(r * cols + c);

                // Right
                if (c + 1 < cols)
                {
                    int right =
                        static_cast<int>(r * cols + c + 1);

                    addEdge(id, right);
                }

                // Down
                if (r + 1 < rows)
                {
                    int down =
                        static_cast<int>((r + 1) * cols + c);

                    addEdge(id, down);
                }

                if (!diagonals)
                    continue;

                // Down-right
                if (r + 1 < rows && c + 1 < cols)
                {
                    int downRight =
                        static_cast<int>(
                            (r + 1) * cols + c + 1
                        );

                    addEdge(id, downRight, 1.4142);
                }

                // Down-left
                if (r + 1 < rows && c > 0)
                {
                    int downLeft =
                        static_cast<int>(
                            (r + 1) * cols + c - 1
                        );

                    addEdge(id, downLeft, 1.4142);
                }
            }
        }
    }


    std::vector<std::vector<int>> findConnectedClusters(
        const std::vector<bool>& active
    ) const
    {
        std::vector<std::vector<int>> clusters;

        if (active.size() !=
            static_cast<size_t>(vertices))
        {
            return clusters;
        }

        std::vector<bool> visited(
            vertices,
            false
        );

        std::queue<int> q;

        for (int start = 0; start < vertices; ++start)
        {
            if (!active[start] || visited[start])
                continue;

            std::vector<int> cluster;

            visited[start] = true;
            q.push(start);

            while (!q.empty())
            {
                int current = q.front();
                q.pop();

                cluster.push_back(current);

                for (const auto& edge : graph[current])
                {
                    int next = edge.id;

                    if (!active[next] ||
                        visited[next])
                    {
                        continue;
                    }

                    visited[next] = true;
                    q.push(next);
                }
            }

            clusters.push_back(std::move(cluster));
        }

        return clusters;
    }
};

}

#endif