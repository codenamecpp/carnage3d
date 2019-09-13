#include "stdafx.h"
#include "PhysicsDebugDraw.h"
#include "RenderingManager.h"

void PhysicsDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
 //   b2Vec2 p1 = vertices[vertexCount - 1];
	//for (int32 i = 0; i < vertexCount; ++i)
	//{
 //       b2Vec2 p2 = vertices[i];
	//	m_lines->Vertex(p1, color);
	//	m_lines->Vertex(p2, color);
 //       p1 = p2;
	//}
}

void PhysicsDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	//b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);

 //   for (int32 i = 1; i < vertexCount - 1; ++i)
 //   {
 //       m_triangles->Vertex(vertices[0], fillColor);
 //       m_triangles->Vertex(vertices[i], fillColor);
 //       m_triangles->Vertex(vertices[i+1], fillColor);
 //   }

 //   b2Vec2 p1 = vertices[vertexCount - 1];
	//for (int32 i = 0; i < vertexCount; ++i)
	//{
 //       b2Vec2 p2 = vertices[i];
	//	m_lines->Vertex(p1, color);
	//	m_lines->Vertex(p2, color);
 //       p1 = p2;
	//}
}

void PhysicsDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	//const float32 k_segments = 16.0f;
	//const float32 k_increment = 2.0f * b2_pi / k_segments;
 //   float32 sinInc = sinf(k_increment);
 //   float32 cosInc = cosf(k_increment);
 //   b2Vec2 r1(1.0f, 0.0f);
 //   b2Vec2 v1 = center + radius * r1;
	//for (int32 i = 0; i < k_segments; ++i)
	//{
 //       // Perform rotation to avoid additional trigonometry.
 //       b2Vec2 r2;
 //       r2.x = cosInc * r1.x - sinInc * r1.y;
 //       r2.y = sinInc * r1.x + cosInc * r1.y;
	//	b2Vec2 v2 = center + radius * r2;
 //       m_lines->Vertex(v1, color);
 //       m_lines->Vertex(v2, color);
 //       r1 = r2;
 //       v1 = v2;
	//}
}
	
void PhysicsDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	//const float32 k_segments = 16.0f;
	//const float32 k_increment = 2.0f * b2_pi / k_segments;
 //   float32 sinInc = sinf(k_increment);
 //   float32 cosInc = cosf(k_increment);
 //   b2Vec2 v0 = center;
 //   b2Vec2 r1(cosInc, sinInc);
 //   b2Vec2 v1 = center + radius * r1;
	//b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	//for (int32 i = 0; i < k_segments; ++i)
	//{
 //       // Perform rotation to avoid additional trigonometry.
 //       b2Vec2 r2;
 //       r2.x = cosInc * r1.x - sinInc * r1.y;
 //       r2.y = sinInc * r1.x + cosInc * r1.y;
	//	b2Vec2 v2 = center + radius * r2;
	//	m_triangles->Vertex(v0, fillColor);
 //       m_triangles->Vertex(v1, fillColor);
 //       m_triangles->Vertex(v2, fillColor);
 //       r1 = r2;
 //       v1 = v2;
	//}

 //   r1.Set(1.0f, 0.0f);
 //   v1 = center + radius * r1;
	//for (int32 i = 0; i < k_segments; ++i)
	//{
 //       b2Vec2 r2;
 //       r2.x = cosInc * r1.x - sinInc * r1.y;
 //       r2.y = sinInc * r1.x + cosInc * r1.y;
	//	b2Vec2 v2 = center + radius * r2;
 //       m_lines->Vertex(v1, color);
 //       m_lines->Vertex(v2, color);
 //       r1 = r2;
 //       v1 = v2;
	//}

 //   // Draw a line fixed in the circle to animate rotation.
	//b2Vec2 p = center + radius * axis;
	//m_lines->Vertex(center, color);
	//m_lines->Vertex(p, color);
}
	
void PhysicsDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	//m_lines->Vertex(p1, color);
	//m_lines->Vertex(p2, color);
}

void PhysicsDebugDraw::DrawTransform(const b2Transform& xf)
{
	//const float32 k_axisScale = 0.4f;
 //   b2Color red(1.0f, 0.0f, 0.0f);
 //   b2Color green(0.0f, 1.0f, 0.0f);
	//b2Vec2 p1 = xf.p, p2;

	//m_lines->Vertex(p1, red);
	//p2 = p1 + k_axisScale * xf.q.GetXAxis();
	//m_lines->Vertex(p2, red);

	//m_lines->Vertex(p1, green);
	//p2 = p1 + k_axisScale * xf.q.GetYAxis();
	//m_lines->Vertex(p2, green);
}

void PhysicsDebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
    //m_points->Vertex(p, color, size);
}