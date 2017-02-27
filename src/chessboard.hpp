#ifndef CHESSBOARD_HEADER
#define CHESSBOARD_HEADER

#include "renderable_entity.hpp"

#include <vec.h>


class Chessboard : public RenderableEntity
{
public:
	Chessboard(): RenderableEntity(){}
	virtual ~Chessboard(){}

	virtual bool init(const int patternWidth, const int patternHeight, const float squareSize)
	{
		static std::vector<float> VERTICES;
	    static std::vector<float> COLORS;
	    static std::vector<float> NORMALS;
	    static std::vector<float> TEXCOORDS;

	    if( VERTICES.empty() )
	    {
	        // done the 1st time only
	        // Normal
	        std::vector<float> n({0,0,-1});

	        float xmax = (float)patternWidth * squareSize;
	        float ymax = (float)patternHeight * squareSize;

	        // prepare vertices coordinates and color
	        for(int i = 0; i < patternWidth; i++){
	            for(int j = 0; j < patternHeight; j++){
	                // Coordinates values
	                float i1 = (i-1)*squareSize;
	                float i2 = i*squareSize;
	                float j1 = (j-1)*squareSize;
	                float j2 = j*squareSize;

	                std::vector<float> p1({i1,j1,0}); // Top left
	                std::vector<float> p2({i2,j1,0}); // Top right
	                std::vector<float> p3({i1,j2,0}); // Bottom left
	                std::vector<float> p4({i2,j2,0}); // Bottom right

	                std::vector<float> t1({0,0}); // Texcoord P1
	                std::vector<float> t2({1,0}); // Texcoord P2
	                std::vector<float> t3({0,1}); // Texcoord P3
	                std::vector<float> t4({1,1}); // Texcoord P4

	                /*    p1 ___ p2
	                 *      |\  |
	                 *      | \ |
	                 *      |__\|
	                 *    p3     p4
	                 */

	                // Top Triangle
	                VERTICES.insert(VERTICES.end(),p1.begin(),p1.end());
	                VERTICES.insert(VERTICES.end(),p4.begin(),p4.end());
	                VERTICES.insert(VERTICES.end(),p2.begin(),p2.end());
	                // Bottom Triangle
	                VERTICES.insert(VERTICES.end(),p1.begin(),p1.end());
	                VERTICES.insert(VERTICES.end(),p3.begin(),p3.end());
	                VERTICES.insert(VERTICES.end(),p4.begin(),p4.end());

	                /////// Texcoords ////////

	                // Top Triangle
	                TEXCOORDS.insert(TEXCOORDS.end(),t1.begin(),t1.end());
	                TEXCOORDS.insert(TEXCOORDS.end(),t4.begin(),t4.end());
	                TEXCOORDS.insert(TEXCOORDS.end(),t2.begin(),t2.end());
	                // Bottom Triangle
	                TEXCOORDS.insert(TEXCOORDS.end(),t1.begin(),t1.end());
	                TEXCOORDS.insert(TEXCOORDS.end(),t3.begin(),t3.end());
	                TEXCOORDS.insert(TEXCOORDS.end(),t4.begin(),t4.end());

	                for(int t = 0; t < 6 ; t++)
	                	NORMALS.insert(NORMALS.end(), n.begin(), n.end());

	                if(i%2 != j%2)  // White
	                {
	                    std::vector<float> c(18,1.f);
	                    COLORS.insert(COLORS.end(),c.begin(),c.end());
	                }
	                else            // Black
	                {
	                    std::vector<float> c(18,0.f);
	                    COLORS.insert(COLORS.end(),c.begin(),c.end());
	                }
	            }
	        }
	    }

  		m_num_vertices = VERTICES.size();
	  	m_useindex = false;
	  	m_usetexture = true;

	  	glGenVertexArrays(1, &m_vao);
	  	glBindVertexArray(m_vao);

	  	glGenBuffers(1, &m_vbo);	  	
	  	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(	GL_ARRAY_BUFFER, 
        				VERTICES.size()*sizeof(VERTICES[0]) + 
        				COLORS.size()*sizeof(COLORS[0])	+
        				NORMALS.size()*sizeof(NORMALS[0]) +
        				TEXCOORDS.size()*sizeof(TEXCOORDS[0]), 
        				0, GL_STATIC_DRAW);


        // Vertex
        int offset = 0;
        int size = VERTICES.size()*sizeof(VERTICES[0]);
  		glBufferSubData(GL_ARRAY_BUFFER, offset, size, VERTICES.data());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(offset));
	  	glEnableVertexAttribArray(0);
  		// Colors
  		offset += size;
  		size = COLORS.size()*sizeof(COLORS[0]);
  		glBufferSubData(GL_ARRAY_BUFFER, offset, size, COLORS.data());
	  	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(offset));
	  	glEnableVertexAttribArray(1);
  		// Normals
  		offset += size;
  		size = NORMALS.size()*sizeof(NORMALS[0]);
  		glBufferSubData(GL_ARRAY_BUFFER, offset, size, NORMALS.data());
	  	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(offset));
	  	glEnableVertexAttribArray(2);
	  	// Texcoords
  		offset += size;
  		size = TEXCOORDS.size()*sizeof(TEXCOORDS[0]);
  		glBufferSubData(GL_ARRAY_BUFFER, offset, size, TEXCOORDS.data());
	  	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(offset));
	  	glEnableVertexAttribArray(3);
	  	  
	  	glBindVertexArray(0);
	  	glBindBuffer(GL_ARRAY_BUFFER, 0);

	  	return true;	
	}
};
#endif