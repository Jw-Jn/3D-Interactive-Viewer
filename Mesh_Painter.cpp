#include "Mesh_Painter.h"
#include "FreeImage.h"
#include <iostream>
using namespace std;
Mesh_Painter::Mesh_Painter()
{
}


Mesh_Painter::~Mesh_Painter()
{
}


void Mesh_Painter::draw_meshes()
{
	for (unsigned int i = 0; i < this->m_my_meshes_.size(); i++)
	{
		glUseProgram(this->program_all[i]);
		glBindVertexArray(this->vao_all[i]);

 		glActiveTexture(GL_TEXTURE0);
 		glBindTexture(GL_TEXTURE_2D, this->textures_all[i]);
		glUniformMatrix4fv(ViewMatrix, 1, GL_TRUE, viewMatrix_all[i]);
		glUniformMatrix4fv(Projection, 1, GL_TRUE, projection_all[i]);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE,   modelView_all[i]);
		glUniform1i(isShadow, 0);
		glUniform3fv(lightPos, 1, &light[0]);
		glDrawArrays(GL_TRIANGLES, 0, this->m_my_meshes_[i]->num_faces() * 3);

		float lx = light[0];
		float ly = light[1];
		float lz = light[2];

		mat4 shadowProjMatrix
		  (-lz, 0.0, 0.0, 0.0,
			0.0, -lz,  0.0 , 0.0,
			lx,ly,  0.0, 1.0,
			0.0, 0.0, 0.0, -lz);

		if (this->m_my_meshes_[i]->hasShadow) {
			glUniformMatrix4fv(ModelView, 1, GL_TRUE, shadowProjMatrix*modelView_all[i]);
			glUniform1i(isShadow, 1);
			glDrawArrays(GL_TRIANGLES, 0, this->m_my_meshes_[i]->num_faces() * 3);
		}

		glUseProgram(0);
	}
};

void Mesh_Painter::set_matrix(mat4 modelViewMatrix, mat4 projection, mat4 modelView, int i) 
{
	viewMatrix_all[i] = modelViewMatrix;
	projection_all[i] = projection;
	modelView_all[i] =  modelView;
}

void Mesh_Painter::update_texture()
{
	this->textures_all.clear();

	for (unsigned int i = 0; i < this->m_my_meshes_.size(); i++)
	{
		GLuint textures;

		glGenTextures(1, &textures);
		
		load_texture_FreeImage(this->m_my_meshes_[i]->get_texture_file(), textures);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, textures);
		glUniform1i(glGetUniformLocation(this->program_all[i], "texture"), 0);
		this->textures_all.push_back(textures);
	}


};


void Mesh_Painter::load_texture_FreeImage(std::string file_name, GLuint& m_texName)
{
	FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(file_name.c_str(), 0);

	FIBITMAP *dib = FreeImage_Load(fifmt, file_name.c_str(), 0);

	dib = FreeImage_ConvertTo24Bits(dib);

	BYTE *pixels = (BYTE*)FreeImage_GetBits(dib);
	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);
	
	glBindTexture(GL_TEXTURE_2D, m_texName);
	
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGB, 
		width,
		height, 
		0, 
		GL_BGR_EXT, 
		GL_UNSIGNED_BYTE, 
		pixels
		);
	
	FreeImage_Unload(dib);
};




void Mesh_Painter::update_vertex_buffer()
{
	this->vao_all.clear();
	this->buffer_all.clear();
	this->vPosition_all.clear();
	this->vColor_all.clear();
	this->vTexCoord_all.clear();
	this->vNormal_all.clear();
	this->viewMatrix_all.clear();
	this->projection_all.clear();
	this->modelView_all.clear();

	for (unsigned int m_i = 0; m_i < this->m_my_meshes_.size(); m_i++)
	{
		std::cout << "no. " << m_i << " mseh" << std :: endl;
		int num_face = this->m_my_meshes_[m_i]->num_faces();
		int num_vertex = this->m_my_meshes_[m_i]->num_vertices();
		std::cout << "num_face" << num_face << std::endl;
		std::cout << "num_vertex" << num_vertex << std::endl;
		viewMatrix_all.push_back(mat4(1.0));
		projection_all.push_back(mat4(1.0));
		modelView_all.push_back(mat4(1.0));

		const VertexList& vertex_list = this->m_my_meshes_[m_i]->get_vertices();
		const NormalList& normal_list = this->m_my_meshes_[m_i]->get_normals();
		const FaceList&  face_list = this->m_my_meshes_[m_i]->get_faces();
		const STLVectorf& color_list = this->m_my_meshes_[m_i]->get_colors();
		const VtList& vt_list = this->m_my_meshes_[m_i]->get_vts();


		// Create a vertex array object
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);


		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vec3)*num_face * 3
			+ sizeof(vec3)*num_face * 3
			+ sizeof(vec3)*num_face * 3
			+ sizeof(vec2)*num_face * 3
			,
			NULL, GL_STATIC_DRAW)
			;

		//   Specify an offset to keep track of where we're placing data in our
		//   vertex array buffer.  We'll use the same technique when we
		//   associate the offsets with vertex attribute pointers.

		vec3* points = new vec3[num_face * 3];
		point3f p_center_ = this->m_my_meshes_[m_i]->get_center();
		point3f p_min_box_, p_max_box_;
		this->m_my_meshes_[m_i]->get_boundingbox(p_min_box_, p_max_box_);
		float d = p_min_box_.distance(p_max_box_);

		for (int i = 0; i < num_face; i++)
		{
			int index = face_list[3 * (3 * i)];
			points[3 * i] = vec3(
				(vertex_list[index * 3 + 0] - p_center_.x),
				(vertex_list[index * 3+ 1] - p_center_.y) ,
				(vertex_list[index * 3+ 2] - p_center_.z)
				);

			index = face_list[3 * (3 * i + 1)];
			points[3 * i + 1] = vec3(
				(vertex_list[index * 3+ 0] - p_center_.x) ,
				(vertex_list[index * 3+ 1] - p_center_.y) ,
				(vertex_list[index * 3+ 2] - p_center_.z) 
				);
			
			index = face_list[3*(3 * i + 2)];
			points[3 * i + 2] = vec3(
				(vertex_list[index * 3+ 0] - p_center_.x) ,
				(vertex_list[index * 3+ 1] - p_center_.y) ,
				(vertex_list[index * 3+ 2] - p_center_.z) 
				);
			
		}
		GLintptr offset = 0;
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec3)*num_face * 3, points);
		std::cout << "loaded vertices" << std::endl;
		
		offset += sizeof(vec3)*num_face * 3;
		delete[] points;
		
		points = new vec3[num_face * 3];
		
		for (int i = 0; i < num_face; i++)
		{
			int index = face_list[3*(3 * i) + 2];
			points[3 * i] = vec3(
				(normal_list[index * 3 + 0]),
				(normal_list[index * 3 + 1]),
				(normal_list[index * 3 + 2])
				);
			
			index = face_list[3* ( 3 * i + 1 ) + 2];
			points[3 * i + 1] = vec3(
				(normal_list[index * 3 + 0]),
				(normal_list[index * 3 + 1]),
				(normal_list[index * 3 + 2])
				);
			
			index = face_list[3* (3 * i + 2) + 2];
			points[3 * i + 2] = vec3(
				(normal_list[index * 3 + 0]),
				(normal_list[index * 3 + 1]),
				(normal_list[index * 3 + 2])
				);
		}
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec3) * num_face * 3, points);
		offset += sizeof(vec3) * num_face * 3;
		delete[] points;
		std::cout << "loaded normals" << std::endl;
		
		points = new vec3[num_face * 3];
		
		for (int i = 0; i < num_face; i++)
		{
			int index = face_list[3*(3 * i) + 2];
			points[3 * i] = vec3(
				(color_list[index * 3 + 0]),
				(color_list[index * 3 + 1]),
				(color_list[index * 3 + 2])
				);
			
			index = face_list[3 *(3 * i + 1) +2];
			points[3 * i + 1] = vec3(
				(color_list[index * 3 + 0]),
				(color_list[index * 3 + 1]),
				(color_list[index * 3 + 2])
				);
			
			index = face_list[3*(3 * i + 2)+2];
			points[3 * i + 2] = vec3(
				(color_list[index * 3 + 0]),
				(color_list[index * 3 + 1]),
				(color_list[index * 3 + 2])
				);
		}
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec3) * num_face * 3, points);
		
		offset += sizeof(vec3) * num_face * 3;
		delete[] points;
		std::cout << "loaded colors" << std::endl;

		vec2* points_2 = new vec2[num_face * 3];
		
		for (int i = 0; i < num_face; i++)
		{
			int index = face_list[3 * (3 * i) + 1];
			points_2[i*3] = vec2(
				(vt_list[index * 2 + 0]),
				(vt_list[index * 2 + 1])
			);
			
			index = face_list[3 * (3 * i + 1) + 1];
			points_2[i*3 + 1] = vec2(
				(vt_list[index * 2 + 0]),
				(vt_list[index * 2 + 1])
			);
			
			index = face_list[3 * (3 * i + 2) + 1];
			points_2[ i*3 + 2] = vec2(
				(vt_list[index * 2 + 0]),
				(vt_list[index * 2 + 1])
			);
		}
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec2) * num_face * 3, points_2);
		offset += sizeof(vec2) * num_face * 3;
		delete points_2;
		std::cout << "loaded textures" << std::endl;

		// Load shaders and use the resulting shader program

		// set up vertex arrays
		offset = 0;
		GLuint vPosition;
		vPosition = glGetAttribLocation(this->program_all[m_i], "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(offset));
		offset += sizeof(vec3) * num_face * 3;
		// specify where begin to load vPosition when in shader

		GLuint vNormal;
		vNormal = glGetAttribLocation(this->program_all[m_i], "vNormal");
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(offset));
		offset += sizeof(vec3) * num_face * 3;
		// specify where begin to load vNormal when in shader

		GLuint vColor;
		vColor = glGetAttribLocation(this->program_all[m_i], "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(offset));
		offset += sizeof(vec3) * num_face * 3;
		// specify where begin to load vColor when in shader

		GLuint vTexCoord;
		vTexCoord = glGetAttribLocation(this->program_all[m_i], "vTexCoord");
		glEnableVertexAttribArray(vTexCoord);
		glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(offset));
		// specify where begin to load vTexCoord when in shader

		ModelView = glGetUniformLocation(this->program_all[m_i], "ModelView");
		Projection = glGetUniformLocation(this->program_all[m_i], "Projection");
		ViewMatrix = glGetUniformLocation(this->program_all[m_i], "ViewMatrix");
		isShadow = glGetUniformLocation(this->program_all[m_i], "isShadow");
		lightPos = glGetUniformLocation(this->program_all[m_i], "lightPos");

		this->vao_all.push_back(vao);
		this->buffer_all.push_back(buffer);
		this->vPosition_all.push_back(vPosition);
		this->vColor_all.push_back(vColor);
		this->vTexCoord_all.push_back(vTexCoord);
		this->vNormal_all.push_back(vNormal);
	}

};
void Mesh_Painter::init_shaders(std::string vs, std::string fs)
{
	this->program_all.clear();
	this->theta_all.clear();
	this->trans_all.clear();
	for (unsigned int i = 0; i < this->m_my_meshes_.size(); i++)
	{
		GLuint program = InitShader(vs.c_str(), fs.c_str());
		this->program_all.push_back(program);
	}

};
void Mesh_Painter::add_mesh(My_Mesh* m)
{
	this->m_my_meshes_.push_back(m);
};
void Mesh_Painter::clear_mehs()
{
	this->m_my_meshes_.clear();

	this->textures_all.clear();
	this->program_all.clear();
	this->vao_all.clear();
	this->buffer_all.clear();
	this->vPosition_all.clear();
	this->vColor_all.clear();
	this->vTexCoord_all.clear();
	this->vNormal_all.clear();
};
