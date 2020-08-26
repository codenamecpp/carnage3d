#pragma once

namespace cxx
{
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

        // normalize angle to range [-180, 180)
        inline void normalize_angle_180()
        {
            mDegrees = fmod(mDegrees + 180.0f, 360.0f);
            if (mDegrees < 0.0f)
            {
                mDegrees += 360.0f;
            }
            mDegrees = mDegrees - 180.0f;
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

} // namespace cxx