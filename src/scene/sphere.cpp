#include "sphere.h"

#include <cmath>

#include "pathtracer/bsdf.h"
#include "util/sphere_drawing.h"

namespace CGL {
namespace SceneObjects {

bool Sphere::test(const Ray &r, double &t1, double &t2) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection test.
  // Return true if there are intersections and writing the
  // smaller of the two intersection times in t1 and the larger in t2.

	float a = dot(r.d, r.d);
	float b = 2 * dot(r.o - o, r.d);
	float c = dot(r.o - o, r.o - o) - r2;
	
	float discriminant = b * b - 4 * a * c;
	
	if (discriminant < 0) {
		return false;
	}
	
	t1 = (-b - sqrt(discriminant)) / (2 * a);
	t2 = (-b + sqrt(discriminant)) / (2 * a);
	
	if (t1 > t2) {
		std::swap(t1, t2);
	}
	
	return true;
}

bool Sphere::has_intersection(const Ray &r) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection.
  // Note that you might want to use the the Sphere::test helper here.

	double t1, t2;
	if (!test(r, t1, t2)) return false;
	double t_hit = t1;
	if (t_hit < r.min_t) t_hit = t2; 
	if (t_hit < r.min_t || t_hit > r.max_t) {
		return false;
	}
	r.max_t = t_hit;
	return true;
}

bool Sphere::intersect(const Ray &r, Intersection *i) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection.
  // Note again that you might want to use the the Sphere::test helper here.
  // When an intersection takes place, the Intersection data should be updated
  // correspondingly.

	double t1, t2;
	if (!test(r, t1, t2)) return false;
	double t_hit = t1;
	if (t_hit < r.min_t) t_hit = t2;

	if (t_hit < r.min_t || t_hit > r.max_t) {
		return false;
	}

	r.max_t = t_hit;

	i->t = t_hit;
	i->primitive = this;
	i->bsdf = get_bsdf();

	Vector3D hit_point = r.o + t_hit * r.d;
	i->n = (hit_point - o).unit();

	return true;
}

void Sphere::draw(const Color &c, float alpha) const {
  Misc::draw_sphere_opengl(o, r, c);
}

void Sphere::drawOutline(const Color &c, float alpha) const {
  // Misc::draw_sphere_opengl(o, r, c);
}

} // namespace SceneObjects
} // namespace CGL
