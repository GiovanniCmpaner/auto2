struct GLRenderPoints
{
	void Create()
	{
		const char* vs = \
        "#version 330\n"
        "uniform mat4 projectionMatrix;\n"
        "layout(location = 0) in vec2 v_position;\n"
        "layout(location = 1) in vec4 v_color;\n"
		"layout(location = 2) in float v_size;\n"
        "out vec4 f_color;\n"
        "void main(void)\n"
        "{\n"
        "	f_color = v_color;\n"
        "	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
		"   gl_PointSize = v_size;\n"
        "}\n";
        
		const char* fs = \
        "#version 330\n"
        "in vec4 f_color;\n"
        "out vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "	color = f_color;\n"
        "}\n";
        
		m_programId = sCreateShaderProgram(vs, fs);
		m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
		m_vertexAttribute = 0;
		m_colorAttribute = 1;
		m_sizeAttribute = 2;
        
		// Generate
		glGenVertexArrays(1, &m_vaoId);
		glGenBuffers(3, m_vboIds);
        
		glBindVertexArray(m_vaoId);
		glEnableVertexAttribArray(m_vertexAttribute);
		glEnableVertexAttribArray(m_colorAttribute);
		glEnableVertexAttribArray(m_sizeAttribute);
        
		// Vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);
        
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);
        
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
		glVertexAttribPointer(m_sizeAttribute, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_sizes), m_sizes, GL_DYNAMIC_DRAW);

		sCheckGLError();
        
		// Cleanup
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
        
		m_count = 0;
	}
    
	void Destroy()
	{
		if (m_vaoId)
		{
			glDeleteVertexArrays(1, &m_vaoId);
			glDeleteBuffers(2, m_vboIds);
			m_vaoId = 0;
		}
        
		if (m_programId)
		{
			glDeleteProgram(m_programId);
			m_programId = 0;
		}
	}
    
	void Vertex(const b2Vec2& v, const b2Color& c, float size)
	{
		if (m_count == e_maxVertices)
			Flush();
        
		m_vertices[m_count] = v;
		m_colors[m_count] = c;
		m_sizes[m_count] = size;
		++m_count;
	}
    
    void Flush()
	{
        if (m_count == 0)
            return;
        
		glUseProgram(m_programId);
        
		float proj[16] = { 0.0f };
		g_camera.BuildProjectionMatrix(proj, 0.0f);
        
		glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);
        
		glBindVertexArray(m_vaoId);
        
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_vertices);
        
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);
        
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(float), m_sizes);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, m_count);
        glDisable(GL_PROGRAM_POINT_SIZE);

		sCheckGLError();
        
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
        
		m_count = 0;
	}
    
	enum { e_maxVertices = 512 };
	b2Vec2 m_vertices[e_maxVertices];
	b2Color m_colors[e_maxVertices];
    float m_sizes[e_maxVertices];

	int32 m_count;
    
	GLuint m_vaoId;
	GLuint m_vboIds[3];
	GLuint m_programId;
	GLint m_projectionUniform;
	GLint m_vertexAttribute;
	GLint m_colorAttribute;
	GLint m_sizeAttribute;
};