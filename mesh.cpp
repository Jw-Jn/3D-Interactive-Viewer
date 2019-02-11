#include "mesh.h"
#include <sstream>
#include <fstream>
#include <iosfwd>
#include <algorithm>
#include <gl/GL.h>
#include <math.h>
#include <algorithm>
#include <iostream>


My_Mesh::My_Mesh()
{

	vTranslation[0] = Theta[0] = 0;
	vTranslation[1] = Theta[1] = 0;
	vTranslation[2] = Theta[2] = 0;
	Theta[0] = 45;
	hasShadow = true;
}


My_Mesh::~My_Mesh()
{

}


void My_Mesh::normal_to_color(float nx, float ny, float nz, float& r, float& g, float& b)
{
	r = float(std::min(std::max(0.5 * (nx + 1.0), 0.0), 1.0));
	g = float(std::min(std::max(0.5 * (ny + 1.0), 0.0), 1.0));
	b = float(std::max(std::max(0.5 * (nz + 1.0), 0.0), 0.0));
};

const VtList&  My_Mesh::get_vts()
{
	return this->m_vt_list_;
};
void My_Mesh::clear_data()
{
	m_vertices_.clear();
	m_normals_.clear();
	m_faces_.clear();
	m_color_list_.clear();
	m_vt_list_.clear();
};
void My_Mesh::get_boundingbox(point3f& min_p, point3f& max_p) const
{
	min_p = this->m_min_box_;
	max_p = this->m_max_box_;
};
const STLVectorf&  My_Mesh::get_colors()
{
	return this->m_color_list_;
};
const VertexList& My_Mesh::get_vertices()
{
	return this->m_vertices_;
};
const NormalList& My_Mesh::get_normals()
{
	return this->m_normals_;
};
const FaceList&   My_Mesh::get_faces()
{
	return this->m_faces_;
};

int My_Mesh::num_faces()
{
	return this->m_faces_.size() / 9;
};

int My_Mesh::num_facesPlante()
{
	return this->m_faces_.size() / 3;
};

int My_Mesh::num_vertices()
{
	return this->m_vertices_.size() / 3;
};

const point3f& My_Mesh::get_center()
{
	return this->m_center_;
};
void My_Mesh::load_obj(std::string obj_File) {
	this->clear_data();
	this->m_center_ = point3f(0, 0, 0);

	double maxx = -3, maxy = -3, maxz = -3;

	std::ifstream fin(obj_File);
	std::string s, head;
	double temp1, temp2,temp3;
	float  r, g, b;
	if (!fin.is_open()) {
		std::cout << "Something Went Wrong When Opening Ojfiles" << std::endl;
	}
	while (getline(fin, s)) {
		if (s.length() < 2)continue;
		if (s[0] == 'v') {
			if (s[1] == 't') { 
				std::istringstream in(s);
				std::string head;
				in >> head >> temp1 >> temp2;
				m_vt_list_.push_back(temp1);
				m_vt_list_.push_back(temp2);
			}
			else if (s[1] == 'n') {
				std::istringstream in(s);
				std::string head;
				in >> head >> temp1 >> temp2 >> temp3;
				m_normals_.push_back(temp1);
				m_normals_.push_back(temp2);
				m_normals_.push_back(temp3);
				My_Mesh::normal_to_color(temp1, temp2, temp3, r, g, b);

				m_color_list_.push_back(r);
				m_color_list_.push_back(g);
				m_color_list_.push_back(b);
			}
			else {
				std::istringstream in(s);
				std::string head;
				in >> head >> temp1 >> temp2 >> temp3;
				m_vertices_.push_back(temp1);
				m_vertices_.push_back(temp2);
				m_vertices_.push_back(temp3);
			}
		}
			else if(s[0]=='f'){
             for(int k=s.size()-1;k>=0;k--){
				 if(s[k]=='/')s[k]=' ';
			 }
             std::istringstream in(s);
             std::string head;
             in>>head;
             int i=0,empt1,empt2,empt3;
             while(i<3) 
             {
				 in >> empt1 >> empt2 >> empt3;
				 m_faces_.push_back(empt1-1);
				 m_faces_.push_back(empt2-1);
				 m_faces_.push_back(empt3-1);
                 i++;
             }
         }
	}

	this->m_min_box_ = point3f(-maxx, -maxy, -maxz);
	this->m_max_box_ = point3f(maxx, maxy, maxz);
	std::cout << "load obj file" << std::endl;
	

}

void My_Mesh::set_matrix(mat4 modelViewMatrix, mat4 projection, mat4 modelView) {
	this->ViewMatrix= modelViewMatrix;
	this->Projection = projection;
	this->ModelView = modelView;
}

void My_Mesh::set_texture_file(std::string s)
{
	this->texture_file_name = s;
};
std::string My_Mesh::get_texture_file()
{
	return this->texture_file_name;
};

void My_Mesh::set_translate(float x, float y, float z)
{
	vTranslation[0] = x;
	vTranslation[1] = y;
	vTranslation[2] = z;

};
void My_Mesh::get_translate(float& x, float& y, float& z)
{
	x = vTranslation[0];
	y = vTranslation[1];
	z = vTranslation[2];
};

void My_Mesh::set_theta(float x, float y, float z)
{
	Theta[0] = x;
	Theta[1] = y;
	Theta[2] = z;
};
void My_Mesh::get_theta(float& x, float& y, float& z)
{
	x = Theta[0];
	y = Theta[1];
	z = Theta[2];
};

void My_Mesh::set_theta_step(float x, float y, float z)
{
	Theta_step[0] = x;
	Theta_step[1] = y;
	Theta_step[2] = z;
};

void My_Mesh::add_theta_step()
{
	Theta[0] = Theta[0] + Theta_step[0];
	Theta[1] = Theta[1] + Theta_step[1];
	Theta[2] = Theta[2] + Theta_step[2];
};