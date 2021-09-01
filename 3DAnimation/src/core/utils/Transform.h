#pragma once
#include <glm/glm.hpp>

class Transform
{

	glm::vec3 Translation;
	glm::vec3 Rotation;
	glm::vec3 Scaling;
	Transform* ParentTransform;
	glm::mat4 ModelMatrix;
public:
	Transform(): Translation(0.f), Rotation(0.f), Scaling(1.0f), ModelMatrix(1.0f){}
	~Transform(){}

	Transform& operator*(const Transform& other)
	{

		return *this;
	}
	/*
	Transform& operator=(Transform const& m);
	
	Transform & operator+=(float s);

	GLM_FUNC_DECL mat<4, 4, T, Q> & operator+=(mat<4, 4, U, Q> const& m);
	template<typename U>
	GLM_FUNC_DECL mat<4, 4, T, Q> & operator-=(U s);
	template<typename U>
	GLM_FUNC_DECL mat<4, 4, T, Q> & operator-=(mat<4, 4, U, Q> const& m);
	template<typename U>
	GLM_FUNC_DECL mat<4, 4, T, Q> & operator*=(U s);
	template<typename U>
	GLM_FUNC_DECL mat<4, 4, T, Q> & operator*=(mat<4, 4, U, Q> const& m);
	template<typename U>
	GLM_FUNC_DECL mat<4, 4, T, Q> & operator/=(U s);
	template<typename U>
	GLM_FUNC_DECL mat<4, 4, T, Q> & operator/=(mat<4, 4, U, Q> const& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool operator==(mat<4, 4, T, Q> const& m1, mat<4, 4, T, Q> const& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool operator!=(mat<4, 4, T, Q> const& m1, mat<4, 4, T, Q> const& m2);*/

};