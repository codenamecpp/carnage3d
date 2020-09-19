#pragma once

#include "math_utils.h"

namespace cxx
{
    struct aabbox2d_t
    {
    public:
        aabbox2d_t() = default;

        // @param minp: Min point, bottom left corner
        // @param maxp: Max point, top right corner
        aabbox2d_t(const glm::vec2& minp, const glm::vec2& maxp)
            : mMin(minp)
            , mMax(maxp)
        {
        }

        // expand box if points is beyond of boundaries
        // @param point: Point
        inline void extend(const glm::vec2& point) 
        {
            extend(point.x, point.y);
        }

        inline void extend(float theX, float theY)
        {
            if (theX < mMin.x) mMin.x = theX; else if (theX > mMax.x) mMax.x = theX;
            if (theY < mMin.y) mMin.y = theY; else if (theY > mMax.y) mMax.y = theY;
        }

        inline void clear()
        {
            mMin = { 9999.0f,  9999.0f};
            mMax = {-9999.0f, -9999.0f};
        }

        // expand box if points is beyond of boundaries
        // @param bbox: Bounding box
        inline void extend(const aabbox2d_t& bbox)
        {
            extend(bbox.mMin);
            extend(bbox.mMax);
        }

        // merge two bounding boxes
        // @param bbox: Bounding box
        inline aabbox2d_t union_with(const aabbox2d_t& bbox) const
        {
            aabbox2d_t boxcurrent (mMin, mMax);
            boxcurrent.extend(bbox.mMin);
            boxcurrent.extend(bbox.mMax);
            return boxcurrent;
        }

        // test whether point in within bounding box
        // @param point: Point
        inline bool contains(const glm::vec2& point) const 
        {
            if ((point.x > mMax.x) || (point.y > mMax.y) || (point.x < mMin.x) || (point.y < mMin.y))
                return false;

            return true;
        }

        // test whether bounding box in within bounding box
        // @param point: Point
	    inline bool contains(const aabbox2d_t& other) const
	    {
            if ((other.mMin.x > mMax.x) || (other.mMin.y > mMax.y) || (other.mMax.x < mMin.x) || (other.mMax.y < mMin.y))
                return false;

            return true;
	    }

	    //! stores all 4 edges of the box into a array
        inline void get_corners(glm::vec2 edges[4]) const
	    {
		    /*
			    Edges are stored in this way:

                  1--------2
                  |        |
                  |        |
                  0--------3
		    */

		    edges[0] = mMin;
            edges[2] = mMax;

		    edges[1].x = mMin.x;
            edges[1].y = mMax.y;

		    edges[3].x = mMax.x;
            edges[3].y = mMin.y;
	    }

        // get box center point
        inline glm::vec2 get_center() const 
        {
            return (mMin + mMax) * 0.5f;
        }
        // dimensions
	    inline float get_height() const { return mMax.y - mMin.y; }
	    inline float get_width () const { return mMax.x - mMin.x; }

    public:
        glm::vec2 mMin; // bottom left corner
        glm::vec2 mMax; // top right corner
    };

    // defines axis aligned bounding box
    struct aabbox_t
    {
    public:
        aabbox_t()
            : mMin( 9999.0f,  9999.0f,  9999.0f)
            , mMax(-9999.0f, -9999.0f, -9999.0f)
        {
        }

        // @param argMinPoint: Min
        // @param argMaxPoint: Max
        aabbox_t(const glm::vec3& argMinPoint, const glm::vec3& argMaxPoint)
            : mMin(argMinPoint)
            , mMax(argMaxPoint)
        {
        }

        // expand box if points is beyond of boundaries
        // @param point: Point
        inline void extend(const glm::vec3& point) 
        {
            extend(point.x, point.y, point.z);
        }

        inline void extend(float theX, float theY, float theZ)
        {
            if (theX < mMin.x) mMin.x = theX; else if (theX > mMax.x) mMax.x = theX;
            if (theY < mMin.y) mMin.y = theY; else if (theY > mMax.y) mMax.y = theY;
            if (theZ < mMin.z) mMin.z = theZ; else if (theZ > mMax.z) mMax.z = theZ;
        }

        inline void clear()
        {
            mMin = {9999.0f,9999.0f,9999.0f};
            mMax = {-9999.0f,-9999.0f,-9999.0f};
        }

        // expand box if points is beyond of boundaries
        // @param bbox: Bounding box
        inline void extend(const aabbox_t& bbox)
        {
            extend(bbox.mMin);
            extend(bbox.mMax);
        }

        // merge two bounding boxes
        // @param bbox: Bounding box
        inline aabbox_t union_with(const aabbox_t& bbox) const
        {
            aabbox_t boxcurrent (mMin, mMax);
            boxcurrent.extend(bbox.mMin);
            boxcurrent.extend(bbox.mMax);
            return boxcurrent;
        }

        // test whether point in within bounding box
        // @param point: Point
        inline bool contains(const glm::vec3& point) const 
        {
            return (point.x >= mMin.x) && (point.x <= mMax.x) && 
                (point.y >= mMin.y) && (point.y <= mMax.y) && 
                (point.z >= mMin.z) && (point.z <= mMax.z);
        }

        // test whether bounding box in within bounding box
        // @param point: Point
	    inline bool contains(const aabbox_t& other) const
	    {
		    return other.mMin.x >= mMin.x &&
			    other.mMax.x <= mMax.x &&
			    other.mMin.y >= mMin.y &&
			    other.mMax.y <= mMax.y &&
			    other.mMin.z >= mMin.z &&
			    other.mMax.z <= mMax.z;
	    }

	    //! stores all 8 edges of the box into a array
	    //! \param edges: Pointer to array of 8 edges
        inline void get_edges(glm::vec3* edges) const
	    {
		    glm::vec3 center = get_center();
		    glm::vec3 corner_delta = (mMax - center);

		    /*
			    Edges are stored in this way:
                  /1--------/3
                 /  |      / |
                /   |     /  |
                5---------7  |
                |   0- - -| -2
                |  /      |  /
                |/        | /
                4---------6/ 
		    */

		    edges[0] = {center.x - corner_delta.x, center.y - corner_delta.y, center.z - corner_delta.z};
		    edges[1] = {center.x - corner_delta.x, center.y + corner_delta.y, center.z - corner_delta.z};
		    edges[2] = {center.x + corner_delta.x, center.y - corner_delta.y, center.z - corner_delta.z};
		    edges[3] = {center.x + corner_delta.x, center.y + corner_delta.y, center.z - corner_delta.z};
		    edges[4] = {center.x - corner_delta.x, center.y - corner_delta.y, center.z + corner_delta.z};
		    edges[5] = {center.x - corner_delta.x, center.y + corner_delta.y, center.z + corner_delta.z};
		    edges[6] = {center.x + corner_delta.x, center.y - corner_delta.y, center.z + corner_delta.z};
		    edges[7] = {center.x + corner_delta.x, center.y + corner_delta.y, center.z + corner_delta.z};
	    }

        // get box center point
        inline glm::vec3 get_center() const 
        {
            return (mMin + mMax) * 0.5f;
        }

        // simple geometry stuff
	    inline float get_height() const { return mMax.y - mMin.y; }
	    inline float get_width() const { return mMax.x - mMin.x; }
	    inline float get_depth() const { return mMax.z - mMin.z; }
        inline float get_surface_area() const 
        { 
            const float multiplication = get_width() * get_height() + get_width() * get_depth() + get_height() * get_depth();
            return 2.0f * multiplication; 
        }

    public:
        glm::vec3 mMin;
        glm::vec3 mMax;
    };

    //////////////////////////////////////////////////////////////////////////

    // defines bounding sphere
    struct bounding_sphere_t
    {
    public:
        bounding_sphere_t() = default;

        // @param argSphereOrigin: Sphere center position
        // @param argSphereRadius: Sphere radius
        bounding_sphere_t(const glm::vec3& argSphereOrigin, float argSphereRadius)
            : mOrigin(argSphereOrigin)
            , mRadius(argSphereRadius)
        {
        }

        // Test whether point is within bounding sphere
        // @param point: Point
        inline bool contains(const glm::vec3& point) const 
        {
	        const glm::vec3 disp {point - mOrigin};
	        float lengthSq = (disp.x * disp.x) + (disp.y * disp.y) + (disp.z * disp.z);
	        return lengthSq < (mRadius * mRadius);
        }

    public:
        glm::vec3 mOrigin;
        float mRadius;
    };

    //////////////////////////////////////////////////////////////////////////

    // defines plane in 3d space
    struct plane3d_t
    {
    public:
        plane3d_t() = default;

        // @param origin: Plane origin
        // @param argNormal: Plane normal
        plane3d_t(const glm::vec3& origin, const glm::vec3& planeNormal)
            : mNormal(planeNormal)
        {
            mDistance = -glm::dot(planeNormal, origin);
        }

        // @param argNormal: Plane normal
        // @param argDistance: Plane distance
        plane3d_t(const glm::vec3& argNormal, float argDistance)
            : mDistance(argDistance)
            , mNormal(argNormal)
        {
        }
        // create plane from triangle points
        // @param ptA, ptB, ptC: Points of triangle
        static plane3d_t from_points(const glm::vec3& ptA, const glm::vec3& ptB, const glm::vec3& ptC)
        {
            const glm::vec3 aNormal = glm::cross(ptB - ptA, ptC - ptA);
            plane3d_t resultPlane 
            { 
                aNormal, -glm::dot(aNormal, ptA) 
            };

            resultPlane.normalize();
            return resultPlane;
        }

        // set normal and distance
        // @param a, b, c: Plane normal
        // @param d: Plane distance
        inline void setup(float a, float b, float c, float d) 
        {
            mNormal.x = a;
            mNormal.y = b;
            mNormal.z = c;
            mDistance = d;
        }
        
        // normalize plane
        inline void normalize()
        {
            auto normalLength = mNormal.length();
            if (normalLength)
            {
	            auto length = 1.0f / normalLength;
                mNormal *= length;
                mDistance *= length;
            }
        }

        // get distance from point to plane
        // returns:
        //  > 0 if the point lies in front of the plane 'p'
        //  < 0 if the point lies behind the plane 'p'
        //    0 if the point lies on the plane 'p'
        //
        // the signed distance from the point to the plane is returned.
        inline float get_distance_from_point(const glm::vec3& argPoint) const 
        {
            return glm::dot(mNormal, argPoint) + mDistance;
        }

    public:
        glm::vec3 mNormal;
        float mDistance;
    };

    //////////////////////////////////////////////////////////////////////////

    // defines ray in 3d space
    struct ray3d_t
    {
    public:
        ray3d_t() = default;
        ray3d_t(const glm::vec3& origin, const glm::vec3& direction)
            : mOrigin(origin)
            , mDirection(direction)
        {
        }

    public:
        glm::vec3 mOrigin;
        glm::vec3 mDirection; // Must be normalized!
    };

    //////////////////////////////////////////////////////////////////////////

    // frustum planes
    enum
    {
        FRUSTUM_PLANE_LEFT,
        FRUSTUM_PLANE_RIGHT,
        FRUSTUM_PLANE_BOTTOM,
        FRUSTUM_PLANE_TOP,
        FRUSTUM_PLANE_NEAR,
        FRUSTUM_PLANE_FAR,
        NUM_FRUSTUM_PLANES
    };

    // defines camera frustum
    struct frustum_t
    {
    public:
        frustum_t() = default;

        // compute frustum planes from view-projection matrix
        // @param clip: Matrix
        void compute_from_viewproj_matrix(const glm::mat4& clip)
        {
            plane3d_t* pPlane = nullptr;

	        pPlane = &mPlanes[FRUSTUM_PLANE_NEAR];
	        pPlane->setup(clip[0].w + clip[0].z, clip[1].w + clip[1].z, clip[2].w + clip[2].z, clip[3].w + clip[3].z);
            pPlane->normalize();

            // Left clipping plane.
            pPlane = &mPlanes[FRUSTUM_PLANE_FAR];
            pPlane->setup(clip[0].w - clip[0].z, clip[1].w - clip[1].z, clip[2].w - clip[2].z, clip[3].w - clip[3].z);
            pPlane->normalize();

            // Left clipping plane.
            pPlane = &mPlanes[FRUSTUM_PLANE_LEFT];
            pPlane->setup(clip[0].w + clip[0].x, clip[1].w + clip[1].x, clip[2].w + clip[2].x, clip[3].w + clip[3].x);
            pPlane->normalize();

            pPlane = &mPlanes[FRUSTUM_PLANE_RIGHT];
            pPlane->setup(clip[0].w - clip[0].x, clip[1].w - clip[1].x, clip[2].w - clip[2].x, clip[3].w - clip[3].x);
            pPlane->normalize();

            pPlane = &mPlanes[FRUSTUM_PLANE_BOTTOM];
            pPlane->setup(clip[0].w + clip[0].y, clip[1].w + clip[1].y, clip[2].w + clip[2].y, clip[3].w + clip[3].y);
            pPlane->normalize();

            pPlane = &mPlanes[FRUSTUM_PLANE_TOP];
            pPlane->setup(clip[0].w - clip[0].y, clip[1].w - clip[1].y, clip[2].w - clip[2].y, clip[3].w - clip[3].y);
            pPlane->normalize();
        }

        // test sphere in frustum
        // @param boundingSphere: Sphere data
        bool contains(const bounding_sphere_t& boundingSphere) const
        {
            for (int iplane = 0; iplane < NUM_FRUSTUM_PLANES; ++iplane)
            {
                if (mPlanes[iplane].get_distance_from_point(boundingSphere.mOrigin) <= -boundingSphere.mRadius)
                    return false;
            }

            return true;
        }

        // test bounding box in frustum
        // @param boundingBox: AABBox data
        bool contains(const aabbox_t& boundingBox) const
        {
            const glm::vec3 c{(boundingBox.mMin + boundingBox.mMax) * 0.5f};

            float sizex = boundingBox.mMax.x - boundingBox.mMin.x;
            float sizey = boundingBox.mMax.y - boundingBox.mMin.y;
            float sizez = boundingBox.mMax.z - boundingBox.mMin.z;

            const glm::vec3 corners[8] =
            {
                {(c.x - sizex * 0.5f), (c.y - sizey * 0.5f), (c.z - sizez * 0.5f)},
                {(c.x + sizex * 0.5f), (c.y - sizey * 0.5f), (c.z - sizez * 0.5f)},
                {(c.x - sizex * 0.5f), (c.y + sizey * 0.5f), (c.z - sizez * 0.5f)},
                {(c.x + sizex * 0.5f), (c.y + sizey * 0.5f), (c.z - sizez * 0.5f)},
                {(c.x - sizex * 0.5f), (c.y - sizey * 0.5f), (c.z + sizez * 0.5f)},
                {(c.x + sizex * 0.5f), (c.y - sizey * 0.5f), (c.z + sizez * 0.5f)},
                {(c.x - sizex * 0.5f), (c.y + sizey * 0.5f), (c.z + sizez * 0.5f)},
                {(c.x + sizex * 0.5f), (c.y + sizey * 0.5f), (c.z + sizez * 0.5f)}
            };

            for (int iplane = 0; iplane < NUM_FRUSTUM_PLANES; ++iplane)
            {
                if (mPlanes[iplane].get_distance_from_point(corners[0]) > 0.0f) continue;
                if (mPlanes[iplane].get_distance_from_point(corners[1]) > 0.0f) continue;
                if (mPlanes[iplane].get_distance_from_point(corners[2]) > 0.0f) continue;
                if (mPlanes[iplane].get_distance_from_point(corners[3]) > 0.0f) continue;
                if (mPlanes[iplane].get_distance_from_point(corners[4]) > 0.0f) continue;
                if (mPlanes[iplane].get_distance_from_point(corners[5]) > 0.0f) continue;
                if (mPlanes[iplane].get_distance_from_point(corners[6]) > 0.0f) continue;
                if (mPlanes[iplane].get_distance_from_point(corners[7]) > 0.0f) continue;

                return false;
            }
            return true;
        }

        // test point in frustum
        // @param pointPosition: Point position
        inline bool contains(const glm::vec3& pointPosition) const
        {
            for (int iplane = 0; iplane < NUM_FRUSTUM_PLANES; ++iplane)
            {
                if (mPlanes[iplane].get_distance_from_point(pointPosition) <= 0.0f)
                    return false;
            }

            return true;
        }
    
    public:
        plane3d_t mPlanes[NUM_FRUSTUM_PLANES];
    };

    // thin float angle wrapper
    struct angle_t final
    {
    public:
        enum class units { degrees, radians };

        angle_t() = default;
        // init angle
        explicit angle_t(float angleValue, units angleUnits)
        {
            if (angleUnits == units::radians)
            {
                mDegrees = glm::degrees(angleValue);
            }
            else
            {
                mDegrees = angleValue;
            }
        }

        // convert angle value to angle_t
        static angle_t from_radians(float angleValue) { return angle_t { angleValue, units::radians }; }
        static angle_t from_degrees(float angleValue) { return angle_t { angleValue, units::degrees }; }

        // convert angle_t to angle value
        inline float to_radians() const { return glm::radians(mDegrees); }
        inline float to_degrees() const { return mDegrees; }

        inline float to_degrees_normalize_180() const
        {
            return normalize_angle_180(mDegrees);
        }

        inline float to_degrees_normalize_360() const 
        { 
            return normalize_angle_360(mDegrees);
        }

        // init angle
        inline void set_angle(float angleValue, units angleUnits)
        {
            if (angleUnits == units::radians)
            {
                mDegrees = glm::degrees(angleValue);
            }
            else
            {
                mDegrees = angleValue;
            }
        }

        // get sin and cos for angle
        // @param outSin: Output sin value
        // @param outCos: Output cos value
        inline void get_sin_cos(float& outSin, float& outCos) const
        {
            float angleRadians = to_radians();

            outSin = sin(angleRadians);
            outCos = cos(angleRadians);
        }

        // clear angle
        inline void set_zero()
        {
            mDegrees = 0.0f;
        }

        // test whether angle is nearly zero
        inline bool is_zero() const { return fabs(mDegrees) < 0.01f; }
        inline bool non_zero() const { return fabs(mDegrees) >= 0.01f; }

        // operators
        inline angle_t operator + (angle_t rhs) const { return from_degrees(mDegrees + rhs.mDegrees); }
        inline angle_t operator - (angle_t rhs) const { return from_degrees(mDegrees - rhs.mDegrees); }
        inline angle_t operator - () const { return from_degrees(-mDegrees); }
        inline angle_t operator * (angle_t rhs) const { return from_degrees(mDegrees * rhs.mDegrees); }
        inline angle_t operator / (angle_t rhs) const { return from_degrees(mDegrees / rhs.mDegrees); }
        inline angle_t operator % (angle_t rhs) const { return from_degrees(fmod(mDegrees, rhs.mDegrees)); }

        inline angle_t& operator %= (angle_t rhs) { mDegrees = fmod(mDegrees, rhs.mDegrees); return *this; }
        inline angle_t& operator *= (angle_t rhs) { mDegrees *= rhs.mDegrees; return *this; }
        inline angle_t& operator /= (angle_t rhs) { mDegrees /= rhs.mDegrees; return *this; }
        inline angle_t& operator += (angle_t rhs) { mDegrees += rhs.mDegrees; return *this; }
        inline angle_t& operator -= (angle_t rhs) { mDegrees -= rhs.mDegrees; return *this; }

        inline bool operator < (angle_t rhs) const { return mDegrees < rhs.mDegrees; }
        inline bool operator > (angle_t rhs) const { return mDegrees > rhs.mDegrees; }
        inline bool operator <= (angle_t rhs) const { return mDegrees <= rhs.mDegrees; }
        inline bool operator >= (angle_t rhs) const { return mDegrees >= rhs.mDegrees; }
        inline bool operator == (angle_t rhs) const { return mDegrees == rhs.mDegrees; }
        inline bool operator != (angle_t rhs) const { return mDegrees != rhs.mDegrees; }

    public:
        float mDegrees = 0.0f;
    };

    // linear interpolation angles
    inline angle_t lerp_angles(angle_t lhs, angle_t rhs, float alpha)
    {
        static const float MaxAngleDegrees = 360.0f;

        float difference = fmod(rhs.mDegrees - lhs.mDegrees, MaxAngleDegrees);
        float shortAngleDistance = fmod(2.0f * difference, MaxAngleDegrees) - difference;
        return angle_t::from_degrees(lhs.mDegrees + shortAngleDistance * alpha);
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

} // namespace cxx