#define _USE_MATH_DEFINES

#include "pathtracer.h"
#include <cmath>
#include "scene/light.h"
#include "scene/sphere.h"
#include "scene/triangle.h"


using namespace CGL::SceneObjects;

namespace CGL {

PathTracer::PathTracer() {
  gridSampler = new UniformGridSampler2D();
  hemisphereSampler = new UniformHemisphereSampler3D();

  tm_gamma = 2.2f;
  tm_level = 1.0f;
  tm_key = 0.18;
  tm_wht = 5.0f;
}

PathTracer::~PathTracer() {
  delete gridSampler;
  delete hemisphereSampler;
}

void PathTracer::set_frame_size(size_t width, size_t height) {
  sampleBuffer.resize(width, height);
  sampleCountBuffer.resize(width * height);
}

void PathTracer::clear() {
  bvh = NULL;
  scene = NULL;
  camera = NULL;
  sampleBuffer.clear();
  sampleCountBuffer.clear();
  sampleBuffer.resize(0, 0);
  sampleCountBuffer.resize(0, 0);
}

void PathTracer::write_to_framebuffer(ImageBuffer &framebuffer, size_t x0,
                                      size_t y0, size_t x1, size_t y1) {
  sampleBuffer.toColor(framebuffer, x0, y0, x1, y1);
}

Vector3D
PathTracer::estimate_direct_lighting_hemisphere(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // For this function, sample uniformly in a hemisphere.

  // Note: When comparing Cornel Box (CBxxx.dae) results to importance sampling, you may find the "glow" around the light source is gone.
  // This is totally fine: the area lights in importance sampling has directionality, however in hemisphere sampling we don't model this behaviour.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);

  // This is the same number of total samples as
  // estimate_direct_lighting_importance (outside of delta lights). We keep the
  // same number of samples for clarity of comparison.
  int num_samples = scene->lights.size() * ns_area_light;
  Vector3D L_out;

  // TODO (Part 3): Write your sampling loop here
  // TODO BEFORE YOU BEGIN
  // UPDATE `est_radiance_global_illumination` to return direct lighting instead of normal shading 
  Vector3D wi_d;
  double pdf;
  for (int s = 0; s < num_samples; s++) {
      Vector3D f = isect.bsdf->sample_f(w_out, &wi_d, &pdf);
      float cosTheta = dot(wi_d, Vector3D(0, 0, 1));  
      Vector3D next_d = o2w * wi_d;
      Vector3D next_o = hit_p;

      Ray r_next(next_o, next_d, 1);
      r_next.min_t = EPS_F;
      Intersection i_next;

      if (cosTheta > 0 && bvh->intersect(r_next, &i_next)) {
          Vector3D L = i_next.bsdf->get_emission();
          L_out += L * f * cosTheta / pdf;
      }
  }
  L_out /= num_samples;

  return L_out;
}

Vector3D
PathTracer::estimate_direct_lighting_importance(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // To implement importance sampling, sample only from lights, not uniformly in
  // a hemisphere.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);
  Vector3D L_out;


  for (auto light : scene->lights) {
      Vector3D L_light(0, 0, 0);
      int num_samples = 0;

      for (int s = 0; s < ns_area_light; s++) {
          Vector3D wi_d;      
          double distToLight; 
          double pdf;        

          Vector3D L = light->sample_L(hit_p, &wi_d, &distToLight, &pdf);

          double cosTheta = dot(wi_d, isect.n);
          if (cosTheta <= 0)
              continue; 

          Ray r_next(hit_p, wi_d, 1);
          r_next.min_t = EPS_F;
          r_next.max_t = distToLight - EPS_F;

          Intersection i_next;
          if (!bvh->intersect(r_next, &i_next)) {
              Vector3D f = isect.bsdf->f(w_out, wi_d);
              L_light += L * f * cosTheta / pdf;
          }
          num_samples++;

          if (light->is_delta_light())
              break;
      }

      if (num_samples > 0)
          L_out += L_light / num_samples;
  }
  return L_out;

}

Vector3D PathTracer::zero_bounce_radiance(const Ray &r,
                                          const Intersection &isect) {
  // TODO: Part 3, Task 2
  // Returns the light that results from no bounces of light



	return isect.bsdf->get_emission();



}

Vector3D PathTracer::one_bounce_radiance(const Ray &r,
                                         const Intersection &isect) {
  // TODO: Part 3, Task 3
  // Returns either the direct illumination by hemisphere or importance sampling
  // depending on `direct_hemisphere_sample`


    
    if (direct_hemisphere_sample) {
        return estimate_direct_lighting_hemisphere(r, isect);
    }
    else {
        return estimate_direct_lighting_importance(r, isect);
    }



}
Vector3D PathTracer::at_least_one_bounce_radiance(const Ray& r,
    const Intersection& isect) {
    Matrix3x3 o2w;
    make_coord_space(o2w, isect.n);
    Matrix3x3 w2o = o2w.T();

    Vector3D hit_p = r.o + r.d * isect.t;
    Vector3D w_out = w2o * (-r.d);

    // Base case: when we have reached the final bounce, return the one bounce radiance.
    if (r.depth <= 1) {
        return one_bounce_radiance(r, isect);
    }

    if (!isAccumBounces) {
        // Non-accumulation mode: do not add the direct radiance until the final bounce.
        // Simply propagate the ray until max_ray_depth is reached.
        Vector3D wi;
        double pdf;
        Vector3D f = isect.bsdf->sample_f(w_out, &wi, &pdf);
        if (pdf <= 0 || f == Vector3D(0, 0, 0)) return Vector3D();
        double cosTheta = wi.z;
        if (cosTheta <= 0) return Vector3D();

        Ray next_ray(hit_p, o2w * wi, std::numeric_limits<double>::infinity(), r.depth - 1);
        next_ray.min_t = EPS_F;

        Intersection next_isect;
        if (bvh->intersect(next_ray, &next_isect)) {
            // Directly return the final bounce radiance without modulating by f.
            return at_least_one_bounce_radiance(next_ray, next_isect);
        }
        else {
            return Vector3D();
        }
    }
    else {
        // Accumulation mode: add the current bounce’s direct lighting
        Vector3D L_out = one_bounce_radiance(r, isect);

        Vector3D wi;
        double pdf;
        Vector3D f = isect.bsdf->sample_f(w_out, &wi, &pdf);
        if (pdf > 0 && !(f == Vector3D(0, 0, 0))) {
            double cosTheta = wi.z;
            if (cosTheta > 0) {
                Ray next_ray(hit_p, o2w * wi, std::numeric_limits<double>::infinity(), r.depth - 1);
                next_ray.min_t = EPS_F;
                Intersection next_isect;
                if (bvh->intersect(next_ray, &next_isect)) {
                    Vector3D next_radiance = at_least_one_bounce_radiance(next_ray, next_isect);
                    L_out += f * next_radiance * cosTheta / pdf;
                }
            }
        }
        return L_out;
    }
}




Vector3D PathTracer::est_radiance_global_illumination(const Ray &r) {
  Intersection isect;
  Vector3D L_out;

  // You will extend this in assignment 3-2.
  // If no intersection occurs, we simply return black.
  // This changes if you implement hemispherical lighting for extra credit.

  // The following line of code returns a debug color depending
  // on whether ray intersection with triangles or spheres has
  // been implemented.
  //
  // REMOVE THIS LINE when you are ready to begin Part 3.
  
  if (!bvh->intersect(r, &isect))
    return L_out;


  L_out = zero_bounce_radiance(r, isect);

  Ray bounce_ray = r;
  if (bounce_ray.depth <= 0) {
      bounce_ray.depth = max_ray_depth;
  }

  L_out += at_least_one_bounce_radiance(bounce_ray, isect);

  return L_out;
}

void PathTracer::raytrace_pixel(size_t x, size_t y) {
  // TODO (Part 1.2):
  // Make a loop that generates num_samples camera rays and traces them
  // through the scene. Return the average Vector3D.
  // You should call est_radiance_global_illumination in this function.

  // TODO (Part 5):
  // Modify your implementation to include adaptive sampling.
  // Use the command line parameters "samplesPerBatch" and "maxTolerance"
    int num_samples = ns_aa;
    Vector2D origin = Vector2D(x, y);
    Vector3D pixel_radiance(0.0);

    for (int i = 0; i < num_samples; i++) {
        Vector2D sample = gridSampler->get_sample();
        Vector2D sample_xy = origin + sample;
        Vector2D normalized_xy(sample_xy.x / sampleBuffer.w,
            sample_xy.y / sampleBuffer.h);
        Ray ray = camera->generate_ray(normalized_xy.x, normalized_xy.y);
        pixel_radiance += est_radiance_global_illumination(ray);
    }

    pixel_radiance /= num_samples;
    sampleBuffer.update_pixel(pixel_radiance, x, y);
    sampleCountBuffer[x + y * sampleBuffer.w] = num_samples;
}

void PathTracer::autofocus(Vector2D loc) {
  Ray r = camera->generate_ray(loc.x / sampleBuffer.w, loc.y / sampleBuffer.h);
  Intersection isect;

  bvh->intersect(r, &isect);

  camera->focalDistance = isect.t;
}

} // namespace CGL
