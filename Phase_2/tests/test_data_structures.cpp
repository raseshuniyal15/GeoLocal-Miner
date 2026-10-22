#include "../src/data_structures/GridArray.h"
#include "../src/data_structures/PriorityQueue.h"
#include "../src/data_structures/LinkedList.h"
#include "../src/data_structures/Graph.h"
#include <iostream>
#include <vector>
#include <cassert>

using namespace GeoLocalMiner;

void testGridArray() {
    std::cout << "[TEST] Data Structures: GridArray 2D Dynamic Container... ";
    GridArray<int> arr(4, 5, 0);
    assert(arr.rows() == 4);
    assert(arr.cols() == 5);
    assert(arr.size() == 20);

    arr(2, 3) = 999;
    assert(arr.at(2, 3) == 999);

    // Bounds checking exception
    bool caught = false;
    try {
        arr.at(5, 5) = 1;
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    std::cout << "PASSED" << std::endl;
}

void testCustomPriorityQueue() {
    std::cout << "[TEST] Data Structures: Custom Binary Max-Heap PriorityQueue... ";
    CustomPriorityQueue<int> pq;
    pq.push(30);
    pq.push(10);
    pq.push(50);
    pq.push(20);
    pq.push(40);

    assert(pq.size() == 5);
    assert(pq.top() == 50);

    auto top3 = pq.extractTopN(3);
    assert(top3.size() == 3);
    assert(top3[0] == 50);
    assert(top3[1] == 40);
    assert(top3[2] == 30);

    // Drain all
    auto all = pq.extractAllSorted();
    assert(all.size() == 5);
    assert(all[0] == 50 && all[1] == 40 && all[2] == 30 && all[3] == 20 && all[4] == 10);
    assert(pq.empty());

    std::cout << "PASSED" << std::endl;
}

void testLinkedList() {
    std::cout << "[TEST] Data Structures: Doubly Linked List (Target Trail)... ";
    LinkedList<std::string> list;
    assert(list.empty());

    list.push_back("Zone #1");
    list.push_back("Zone #2");
    list.push_back("Zone #3");
    list.push_front("Zone #0");

    assert(list.size() == 4);
    assert(list[0] == "Zone #0");
    assert(list[1] == "Zone #1");
    assert(list[2] == "Zone #2");
    assert(list[3] == "Zone #3");

    // Remove middle
    list.removeAt(1); // removes "Zone #1"
    assert(list.size() == 3);
    assert(list[0] == "Zone #0");
    assert(list[1] == "Zone #2");

    // Copy semantics
    LinkedList<std::string> copy = list;
    assert(copy.size() == 3);
    assert(copy[0] == "Zone #0");

    list.clear();
    assert(list.empty());
    assert(!copy.empty()); // Deep copy intact

    std::cout << "PASSED" << std::endl;
}

void testSpatialGraph() {
    std::cout << "[TEST] Data Structures: SpatialGraph Adjacency & BFS Clusters... ";
    SpatialGraph graph;
    // Build 3x3 grid topology
    graph.buildGridTopology(3, 3, false); // 4-way orthogonal
    assert(graph.vertexCount() == 9);

    // Corner cell [0, 0] (id=0) should have degree 2: right [0, 1] and down [1, 0]
    assert(graph.getDegree(0) == 2);
    // Center cell [1, 1] (id=4) should have degree 4
    assert(graph.getDegree(4) == 4);

    // Test BFS Connected Components (Contiguous Prospectivity Clusters)
    // Mark cell 0, 1, 4 as high priority (contiguous cluster of 3)
    // Mark cell 8 as separate high priority isolated cell (cluster of 1)
    std::vector<bool> highMask(9, false);
    highMask[0] = true;
    highMask[1] = true;
    highMask[4] = true;
    highMask[8] = true;

    auto clusters = graph.findConnectedClusters(highMask);
    assert(clusters.size() == 2);
    // Cluster 1 has 3 cells, Cluster 2 has 1 cell
    size_t sz1 = clusters[0].size();
    size_t sz2 = clusters[1].size();
    assert((sz1 == 3 && sz2 == 1) || (sz1 == 1 && sz2 == 3));

    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "=== RUNNING TEST SUITE: DATA STRUCTURES (PBL REQUIREMENTS) ===" << std::endl;
    testGridArray();
    testCustomPriorityQueue();
    testLinkedList();
    testSpatialGraph();
    std::cout << ">>> ALL DATA STRUCTURE TESTS PASSED! <<<\n" << std::endl;
    return 0;
}
