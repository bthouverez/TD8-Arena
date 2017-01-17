#ifndef SCREENQUAD_HEADER
#define SCREENQUAD_HEADER


class ScreenQuad : public RenderableEntity
{
public:
	ScreenQuad(): RenderableEntity(){}
	virtual ~ScreenQuad(){}

	virtual bool init()
	{
		const float vertices[18] = {
			-1.0, -1.0, 0.0, 
			1.0, -1.0, 0.0, 
			1.0, 1.0, 0.0, 

			-1.0, 1.0, 0.0, 
			1.0, 1.0, 0.0, 
			-1.0, 1.0, 0.0 };
		const float texcoords[12] = {
			0.0, 0.0,
			1.0, 0.0, 
			1.0, 1.0, 

			0.0, 0.0, 
			1.0, 1.0, 
			0.0, 1.0 };

		m_num_vertices = 6;

  	
  	m_useindex = false;
  	m_usetexture = true;

  	glGenVertexArrays(1, &m_vao);
  	glBindVertexArray(m_vao);

  	glGenBuffers(1, &m_vbo);
  	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  	glBufferData(GL_ARRAY_BUFFER, m_num_vertices * sizeof(vec3) + m_num_vertices *sizeof(vec2), 0, GL_STATIC_DRAW);
  	glBufferSubData(GL_ARRAY_BUFFER, 0, m_num_vertices * sizeof(vec3), vertices);//coords
  	glBufferSubData(GL_ARRAY_BUFFER, m_num_vertices *sizeof(vec3), m_num_vertices*sizeof(vec2), texcoords);//texcoords
  	  
  	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);//coords
  	glEnableVertexAttribArray(0);
  	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(m_num_vertices*sizeof(vec3)));//texcoords
  	glEnableVertexAttribArray(1);
  	  
  	glBindVertexArray(0);
  	glBindBuffer(GL_ARRAY_BUFFER, 0);

  	return true;	
	}
};
#endif