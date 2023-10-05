#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include "Particle.h"

// Define the boundary for each node in the QuadTree
class Boundary {
public:
    double x, y, width, height;

    Boundary(double x, double y, double w, double h);

    bool contains(const Particle& particle) const;
    bool intersects(const Boundary& other) const;
};

// Node class for QuadTree
class QuadTreeNode {
public:
    QuadTreeNode(Boundary boundary, size_t capacity);
    ~QuadTreeNode();

    // Operations
    bool insert(Particle* particle);
    void subdivide();

    // Fetch all particles in the boundary
    void query(const Boundary& range, std::vector<Particle*>& found) const;

    // Attributes
    Boundary boundary;
    size_t capacity;
    std::vector<Particle*> particles;

    QuadTreeNode* northwest = nullptr;
    QuadTreeNode* northeast = nullptr;
    QuadTreeNode* southwest = nullptr;
    QuadTreeNode* southeast = nullptr;
};

// Main QuadTree class
class QuadTree {
public:
    QuadTree(Boundary boundary, size_t capacity);
    ~QuadTree();

    void insert(Particle* particle);
    void clear();
    void query(const Boundary& range, std::vector<Particle*>& found) const;

private:
    QuadTreeNode* root;
};

#endif // QUADTREE_H
