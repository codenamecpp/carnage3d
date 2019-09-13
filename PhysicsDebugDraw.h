#pragma once

// debug draw class for box2d physics engine
class PhysicsDebugDraw: public b2Draw
{
public:
	/// Draw a closed polygon provided in CCW order.
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	/// Draw a solid closed polygon provided in CCW order.
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	/// Draw a circle.
	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override;
	
	/// Draw a solid circle.
	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override;
	
	/// Draw a line segment.
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

	/// Draw a transform. Choose your own length scale.
	/// @param xf a transform.
	void DrawTransform(const b2Transform& xf) override;

	/// Draw a point.
	void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override;
};