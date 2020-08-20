struct Camera
{
	Camera()
	{
		m_center.Set(0.0f, 20.0f);
		m_zoom = 1.0f;
		m_width = 1280;
		m_height = 800;
	}

	b2Vec2 ConvertScreenToWorld(const b2Vec2& screenPoint);
	b2Vec2 ConvertWorldToScreen(const b2Vec2& worldPoint);
	void BuildProjectionMatrix(float* m, float zBias);

	b2Vec2 m_center;
	float m_zoom;
	int32 m_width;
	int32 m_height;
};