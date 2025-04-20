#include "bvh.h"

#include "CGL/CGL.h"
#include "triangle.h"

#include <iostream>
#include <stack>

using namespace std;

namespace CGL {
namespace SceneObjects {

BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                   size_t max_leaf_size) {

  primitives = std::vector<Primitive *>(_primitives);
  root = construct_bvh(primitives.begin(), primitives.end(), max_leaf_size);
}

BVHAccel::~BVHAccel() {
  if (root)
    delete root;
  primitives.clear();
}

BBox BVHAccel::get_bbox() const { return root->bb; }

void BVHAccel::draw(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->draw(c, alpha);
    }
  } else {
    draw(node->l, c, alpha);
    draw(node->r, c, alpha);
  }
}

void BVHAccel::drawOutline(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->drawOutline(c, alpha);
    }
  } else {
    drawOutline(node->l, c, alpha);
    drawOutline(node->r, c, alpha);
  }
}

BVHNode* BVHAccel::construct_bvh(std::vector<Primitive*>::iterator start,
    std::vector<Primitive*>::iterator end,
    size_t max_leaf_size) {
    // 1. Compute bounding box over all primitives
    //    and also prepare to compute the average centroid
    BBox bbox;
    Vector3D centroid_sum(0, 0, 0);
    size_t nPrims = end - start;

    for (auto p = start; p != end; p++) {
        BBox prim_bbox = (*p)->get_bbox();
        bbox.expand(prim_bbox);
        centroid_sum += prim_bbox.centroid();
    }

    // Create new BVHNode
    BVHNode* node = new BVHNode(bbox);

    // 2. Base case: create leaf if below max_leaf_size
    if (nPrims <= max_leaf_size) {
        node->start = start;
        node->end = end;
        node->l = nullptr;
        node->r = nullptr;
        return node;
    }

    // 3. Compute average centroid of the primitives in [start, end)
    Vector3D centroid_mean = centroid_sum / (double)nPrims;

    // 4. We’ll pick whichever axis gives us the smallest bounding-box “cost” for left vs. right.
    //    For each axis in {0,1,2}, partition around centroid_mean[axis] to get left & right sets,
    //    compute surface area of each side times the number of primitives, and pick the axis 
    //    with the smallest cost.
    float minCost = std::numeric_limits<float>::infinity();
    int best_axis = 0;
    // We’ll store the partition boundary (split pos) along that axis as well.
    // In this method, we use centroid_mean[axis], just like the second snippet.
    // If you prefer to test a range of positions, you can expand the logic here.
    for (int axis = 0; axis < 3; axis++) {

        // Partition into left and right sets if centroid <= mean or > mean on that axis
        std::vector<Primitive*> left_prims;
        std::vector<Primitive*> right_prims;

        left_prims.reserve(nPrims);
        right_prims.reserve(nPrims);

        double split_pos = centroid_mean[axis];

        for (auto p = start; p != end; p++) {
            // Compare centroid to split_pos
            double c = (*p)->get_bbox().centroid()[axis];
            if (c <= split_pos) {
                left_prims.push_back(*p);
            }
            else {
                right_prims.push_back(*p);
            }
        }

        // If everything ended up on one side, cost is effectively infinite 
        // (which means we skip this axis).
        if (left_prims.empty() || right_prims.empty()) {
            continue;
        }

        // Compute bounding boxes for left and right partitions
        BBox left_bbox, right_bbox;
        for (auto p : left_prims) {
            left_bbox.expand(p->get_bbox());
        }
        for (auto p : right_prims) {
            right_bbox.expand(p->get_bbox());
        }

        // “Cost” = (#left * surfaceArea(left)) + (#right * surfaceArea(right)).
        // Surface area of a BBox = 2*(dx*dy + dy*dz + dz*dx).
        auto leftExtent = left_bbox.extent;
        auto rightExtent = right_bbox.extent;
        float leftSA = 2.f * (leftExtent.x * leftExtent.y +
            leftExtent.y * leftExtent.z +
            leftExtent.z * leftExtent.x);
        float rightSA = 2.f * (rightExtent.x * rightExtent.y +
            rightExtent.y * rightExtent.z +
            rightExtent.z * rightExtent.x);

        float cost = (float)(left_prims.size()) * leftSA +
            (float)(right_prims.size()) * rightSA;

        if (cost < minCost) {
            minCost = cost;
            best_axis = axis;
        }
    }

    // 5. If we never found a valid axis that splits anything, or everything ended on one side,
    //    just do a fallback: pick, e.g., the mid-point in [start, end). This prevents degenerate 
    //    partitions from recurring.
    if (minCost == std::numeric_limits<float>::infinity()) {
        // Fallback: split in half by count
        auto mid_ptr = start + (nPrims / 2);
        node->l = construct_bvh(start, mid_ptr, max_leaf_size);
        node->r = construct_bvh(mid_ptr, end, max_leaf_size);

        // Clear node->start, node->end for an internal node
        node->start = std::vector<Primitive*>::iterator();
        node->end = std::vector<Primitive*>::iterator();
        return node;
    }

    // 6. Otherwise, we have a best_axis. We do the actual partition along best_axis using the 
    //    centroid_mean. Then we build child nodes recursively.
    double best_split_pos = centroid_mean[best_axis];
    auto mid_ptr = std::partition(start, end, [&](Primitive* p) {
        double c = p->get_bbox().centroid()[best_axis];
        return (c <= best_split_pos);
        });

    // Again, if partition fails (everything in one side), fallback to splitting by count:
    if (mid_ptr == start || mid_ptr == end) {
        mid_ptr = start + (nPrims / 2);
    }

    node->l = construct_bvh(start, mid_ptr, max_leaf_size);
    node->r = construct_bvh(mid_ptr, end, max_leaf_size);

    // Clear node->start, node->end for an internal node
    node->start = std::vector<Primitive*>::iterator();
    node->end = std::vector<Primitive*>::iterator();

    return node;
}

bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  // Take note that this function has a short-circuit that the
  // Intersection version cannot, since it returns as soon as it finds
  // a hit, it doesn't actually have to find the closest hit.


    if (!node) return false;

    double t0 = ray.min_t;
    double t1 = ray.max_t;
    if (!node->bb.intersect(ray, t0, t1)) {
        return false;
    }

    if (node->isLeaf()) {
        for (auto p = node->start; p != node->end; p++) {
            total_isects++; 
            if ((*p)->has_intersection(ray)) {
                return true;  
            }
        }
        return false;
    }

    if (has_intersection(ray, node->l)) {
        return true;
    }
    return has_intersection(ray, node->r);


}

bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.

    if (!node) return false;

    double t0 = ray.min_t;
    double t1 = ray.max_t;
    if (!node->bb.intersect(ray, t0, t1)) {
        return false;
    }

    bool hit = false;
    if (node->isLeaf()) {
        for (auto p = node->start; p != node->end; p++) {
            total_isects++; 
            // If a primitive intersects, it updates 'i' with the closest intersection
            if ((*p)->intersect(ray, i)) {
                hit = true;
            }
        }
        return hit;
    }
    bool hitLeft = intersect(ray, i, node->l);
    bool hitRight = intersect(ray, i, node->r);
    return hitLeft || hitRight;

}

} // namespace SceneObjects
} // namespace CGL
