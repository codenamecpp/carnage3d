#pragma once

namespace cxx
{
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

} // namespace cxx