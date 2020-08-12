#pragma once

#ifndef FLT_EPSILON
    #define FLT_EPSILON 0.0000001f
#endif

namespace cxx
{
    // get next power of two value
    inline unsigned int get_next_pot(unsigned int value)
    {
        if (value > 0)
        {
            --value;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            ++value;
        }
        return value;
    }

    // test number is power of two
    inline bool is_pot(unsigned int value)
    {
        return value && !(value & (value - 1));
    }

    // test is number even or odd
    inline bool is_even(unsigned int value) { return (value % 2) == 0; }
    inline bool is_odd(unsigned int value)  { return (value % 2)  > 0; }

    // normalize angle
    inline float normalize_angle_180(float angleDegrees)
    {
        angleDegrees = std::fmod(angleDegrees + 180.0f, 360.0f);
        if (angleDegrees < 0.0f)
        {
            angleDegrees += 360.0f;
        }
        return angleDegrees - 180.0f;
    }

    // round up value
    inline unsigned int round_up_to(unsigned int value, unsigned int pad)
    {
        if (pad > 0 && (value % pad))
        {
            const unsigned int value_to_add = pad - (value % pad);
            return value + value_to_add;
        }
        return value;
    }

    inline unsigned int align_up(unsigned int value, unsigned int alignment)
    {
        if (alignment > 0)
        {
            value = ((value) + (alignment - 1)) & ~(alignment - 1);
        }
        return value;
    }

    inline unsigned int aligh_down(unsigned int value, unsigned int alignment)
    {
        if (alignment > 0)
        {
            value = value & ~(alignment - 1);
        }
        return value;
    }

    inline bool is_aligned(unsigned int value, unsigned alignment)
    {
        return ((value) & (alignment - 1)) == 0;
    }

    // create a new interpolated bounding box
    // @param interpolation: value between 0.0f and 1.0f.
    inline aabbox_t interpolate_aabbox(const aabbox_t& aaboxA, const aabbox_t& aaboxB, float interpolation)
    {
	    float inv = 1.0f - interpolation;
	    return {
            (aaboxB.mMin * inv) + (aaboxA.mMin * interpolation),
		    (aaboxB.mMax * inv) + (aaboxA.mMax * interpolation)};
    }

    // transform aabox
    // @param aabox: Source aabox
    // @param transformation: Transformation matrix
    inline aabbox_t transform_aabbox(const aabbox_t& aabox, const glm::mat4& tmat)
    {
 	    // Efficient algorithm for transforming an AABB, taken from Graphics Gems
		
        aabbox_t transformedAabb;

	    for (int i = 0; i < 3; ++i)
	    {
		    transformedAabb.mMin[i] = tmat[3][i];
		    transformedAabb.mMax[i] = tmat[3][i];
			
		    for (int j = 0; j < 3; ++j)
		    {
			    float x = aabox.mMin[j] * tmat[j][i];
			    float y = aabox.mMax[j] * tmat[j][i];
			    transformedAabb.mMin[i] += fminf(x, y);
			    transformedAabb.mMax[i] += fmaxf(x, y);
		    }
	    }
        return transformedAabb;
    }

    // Create bounding sphere from aabbox
    // @param aabb: Bounding box
    inline bounding_sphere_t compute_bounding_sphere(const aabbox_t& aabb)
    {
        bounding_sphere_t sphere {
            // origin
            (aabb.mMin + aabb.mMax) * 0.5f,
            // radius
            (aabb.mMax - aabb.mMin).length() * 0.5f
        };
        return sphere;
    }

    // test intersection between two aabboxes
    // @param bboxA, bboxB: Bounding boxes
    inline bool intersects(const aabbox_t& bboxA, const aabbox_t& bboxB) 
    {
        if (bboxA.mMax.x < bboxB.mMin.x || bboxA.mMin.x > bboxB.mMax.x ||
            bboxA.mMax.y < bboxB.mMin.y || bboxA.mMin.y > bboxB.mMax.y ||
            bboxA.mMax.z < bboxB.mMin.z || bboxA.mMin.z > bboxB.mMax.z)
        {
            return false;
        }

        return true;
    }

    // test intersection aabox vs ray
    // @param bbox: Bouding box
    // @param ray3d: Ray
    // @param distanceNear, distanceFar: Intersection distance near/far
    inline bool intersects(const aabbox_t& bbox, const ray3d_t& theRay, float& distanceNear, float& distanceFar)
    {
        // Intermediate calculation variables.
        float dnear = 0.0f;
        float dfar = 0.0f;
        float tmin = 0.0f;
        float tmax = 0.0f;

        // X direction.
        float div = 1.0f / theRay.mDirection.x;
        if (div >= 0.0f)
        {
            tmin = (bbox.mMin.x - theRay.mOrigin.x) * div;
            tmax = (bbox.mMax.x - theRay.mOrigin.x) * div;
        }
        else
        {
            tmin = (bbox.mMax.x - theRay.mOrigin.x) * div;
            tmax = (bbox.mMin.x - theRay.mOrigin.x) * div;
        }
        dnear = tmin;
        dfar = tmax;

        // Check if the ray misses the box.
        if (dnear > dfar || dfar < 0.0f)
        {
            return false;
        }

        // Y direction.
        div = 1.0f / theRay.mDirection.y;
        if (div >= 0.0f)
        {
            tmin = (bbox.mMin.y - theRay.mOrigin.y) * div;
            tmax = (bbox.mMax.y - theRay.mOrigin.y) * div;
        }
        else
        {
            tmin = (bbox.mMax.y - theRay.mOrigin.y) * div;
            tmax = (bbox.mMin.y - theRay.mOrigin.y) * div;
        }

        // Update the near and far intersection distances.
        if (tmin > dnear)
        {
            dnear = tmin;
        }
        if (tmax < dfar)
        {
            dfar = tmax;
        }
        // Check if the ray misses the box.
        if (dnear > dfar || dfar < 0.0f)
        {
            return false;
        }

        // Z direction.
        div = 1.0f / theRay.mDirection.z;
        if (div >= 0.0f)
        {
            tmin = (bbox.mMin.z - theRay.mOrigin.z) * div;
            tmax = (bbox.mMax.z - theRay.mOrigin.z) * div;
        }
        else
        {
            tmin = (bbox.mMax.z - theRay.mOrigin.z) * div;
            tmax = (bbox.mMin.z - theRay.mOrigin.z) * div;
        }

        // Update the near and far intersection distances.
        if (tmin > dnear)
        {
            dnear = tmin;
        }
        if (tmax < dfar)
        {
            dfar = tmax;
        }

        // Check if the ray misses the box.
        if (dnear > dfar || dfar < 0.0f)
        {
            return false;
        }
        // The ray intersects the box (and since the direction of a Ray is normalized, dnear is the distance to the ray).
        distanceNear = dnear;
        distanceFar = dfar;
        return true;
    }

    // test intersection sphere vs ray
    // @param theSphere: Bouding sphere
    // @param ray3d: Ray
    // @param theDistance: Intersection distance
    inline bool intersects(const bounding_sphere_t& theSphere, const ray3d_t& theRay, float& theDistance)
    {
        const glm::vec3 centerVec = theSphere.mOrigin - theRay.mOrigin;
        const float vecProjection = glm::dot(centerVec, theRay.mDirection);
        const float sqDistance = glm::length2(centerVec);
        const float sqRadius = theSphere.mRadius * theSphere.mRadius;

        // Sphere behind the ray origin and ray origin outside the sphere
        if (vecProjection < 0 && sqDistance > sqRadius)
            return false;

        // Squared distance from sphere center to the projection
        const float sqCenterToProj = sqDistance - vecProjection * vecProjection;
        if (sqCenterToProj > sqRadius)
            return false;

        // Distance from the sphere center to the surface along the ray direction
        const float distSurface = sqrt(sqRadius - sqCenterToProj);
        if(sqDistance > sqRadius)
        {
            theDistance = vecProjection - distSurface;
        }
        else
        {
            theDistance = vecProjection + distSurface;
        }
        return true;
    }

    // http://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
    //// r.dir is unit direction vector of ray
    //dirfrac.x = 1.0f / r.dir.x;
    //dirfrac.y = 1.0f / r.dir.y;
    //dirfrac.z = 1.0f / r.dir.z;
    //// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    //// r.org is origin of ray
    //float t1 = (lb.x - r.org.x)*dirfrac.x;
    //float t2 = (rt.x - r.org.x)*dirfrac.x;
    //float t3 = (lb.y - r.org.y)*dirfrac.y;
    //float t4 = (rt.y - r.org.y)*dirfrac.y;
    //float t5 = (lb.z - r.org.z)*dirfrac.z;
    //float t6 = (rt.z - r.org.z)*dirfrac.z;
    //
    //float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    //float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));
    //
    //// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    //if (tmax < 0)
    //{
    //    t = tmax;
    //    return false;
    //}
    //
    //// if tmin > tmax, ray doesn't intersect AABB
    //if (tmin > tmax)
    //{
    //    t = tmax;
    //    return false;
    //}
    //
    //t = tmin;
    //return true;

    // test intersection ray versus triangle
    // @param ray3d: Ray
    // @param tv0, tv1, tv2: Triangle vertices
    // @param outPoint: Intersetion point if success
    // Moller-Trumbore algorithm 
    inline bool intersects(const ray3d_t& ray3d, const glm::vec3& vertex0, const glm::vec3& vertex1, const glm::vec3& vertex2, glm::vec3& outPoint)
    {
        glm::vec3 edge1 = vertex1 - vertex0;
        glm::vec3 edge2 = vertex2 - vertex0;
        glm::vec3 pvec = glm::cross(ray3d.mDirection, edge2);
        float det = glm::dot(edge1, pvec);
        if (det > -FLT_EPSILON && det < FLT_EPSILON)
            return false;    // This ray is parallel to this triangle

        float inv_det = 1.0f / det;

        glm::vec3 tvec = ray3d.mOrigin - vertex0;
        float u = inv_det * glm::dot(tvec, pvec);
        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 qvec = glm::cross(tvec, edge1);
        float v = inv_det * glm::dot(ray3d.mDirection, qvec);
        if (v < 0.0f || u + v > 1.0f)
            return false;

        // At this stage we can compute t to find out where the intersection point is on the line
        float t = inv_det * glm::dot(edge2, qvec);
        if (t > FLT_EPSILON) // ray intersection
        {
            outPoint = ray3d.mOrigin + ray3d.mDirection * t;
            return true;
        }
        // This means that there is a line intersection but not a ray intersection
        return false;
    }

    // loops the value within range [0, 1]
    inline float repeat(float value, float length)
    {
        return value - glm::floor(value / length) * length;
    }

    // ping-pongs the value in range [0, 1]
    inline float ping_pong(float value, float length)
    {
        value = repeat(value, length * 2.0f);
        return length - glm::abs(value - length);
    }

} // namespace cxx