#include "Quadtree.h"
#include "Particle.h"
#include "HugoStable.h"

// Implementations for Boundary
Boundary::Boundary(double x, double y, double w, double h) : x(x), y(y), width(w), height(h) {}

bool Boundary::contains(const Particle& particle) const {
    return (particle.getX() >= x - width &&
            particle.getX() <= x + width &&
            particle.getY() >= y - height &&
            particle.getY() <= y + height);
}

bool Boundary::intersects(const Boundary& other) const {
    return !(other.x - other.width > x + width ||
             other.x + other.width < x - width ||
             other.y - other.height > y + height ||
             other.y + other.height < y - height);
}

// Implementations for QuadTreeNode
QuadTreeNode::QuadTreeNode(Boundary boundary, size_t capacity) : boundary(boundary), capacity(capacity) {}

QuadTreeNode::~QuadTreeNode() {
    delete northwest;
    delete northeast;
    delete southwest;
    delete southeast;
}

bool QuadTreeNode::insert(Particle* particle) {
    if (!boundary.contains(*particle)) return false;
    
    if (particles.size() < capacity) {
        particles.push_back(particle);
        return true;
    }
    
    if (!northwest) {
        subdivide();
    }

    if (northwest->insert(particle)) return true;
    if (northeast->insert(particle)) return true;
    if (southwest->insert(particle)) return true;
    if (southeast->insert(particle)) return true;

    return false;
}

void QuadTreeNode::subdivide() {
    double x = boundary.x;
    double y = boundary.y;
    double halfW = boundary.width / 2;
    double halfH = boundary.height / 2;

    northwest = new QuadTreeNode(Boundary(x - halfW, y - halfH, halfW, halfH), capacity);
    northeast = new QuadTreeNode(Boundary(x + halfW, y - halfH, halfW, halfH), capacity);
    southwest = new QuadTreeNode(Boundary(x - halfW, y + halfH, halfW, halfH), capacity);
    southeast = new QuadTreeNode(Boundary(x + halfW, y + halfH, halfW, halfH), capacity);
}

void QuadTreeNode::query(const Boundary& range, std::vector<Particle*>& found) const {
    if (!boundary.intersects(range)) return;

    for (Particle* particle : particles) {
        if (range.contains(*particle)) {
            found.push_back(particle);
        }
    }

    if (!northwest) return;

    northwest->query(range, found);
    northeast->query(range, found);
    southwest->query(range, found);
    southeast->query(range, found);
}

// Implementations for QuadTree
QuadTree::QuadTree(Boundary boundary, size_t capacity) {
    root = new QuadTreeNode(boundary, capacity);
}

QuadTree::~QuadTree() {
    delete root;
}

void QuadTree::insert(Particle* particle) {
    root->insert(particle);
}

void QuadTree::clear() {
    delete root;
    root = new QuadTreeNode(root->boundary, root->capacity);
}

void QuadTree::query(const Boundary& range, std::vector<Particle*>& found) const {
    root->query(range, found);
}
