#pragma once

#ifndef COLLADA_PARSER_HPP
#define COLLADA_PARSER_HPP
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "app/JointAnimation.h"

struct Joint;


class ColladaParser
{
	xmlDocPtr XmlDocument;
	xmlNode* Root;
	xmlNode* LibraryVisualScenes;
	xmlNode* LibraryControllers;
	xmlNode* Armature;
	xmlNode* RootJoint;
	Joint* SkeletonRoot;
	std::vector<JointAnimation> Animation;
	bool ApplyAxisCorrection;

public:

	ColladaParser();
	//movable
	ColladaParser(ColladaParser&& other);
	ColladaParser& operator=(ColladaParser&& other);
	//non copiable
	ColladaParser(const ColladaParser&) = delete;
	ColladaParser& operator=(const ColladaParser& other) = delete;


	~ColladaParser();

	Joint* GetJointHerarchy(const char* path,bool ApplyAxisCorrection = false);
	std::vector<JointAnimation> GetAnimation(const char* filepath);

private:

	Joint* ParseSkeleton(xmlNode* node, Joint* parent,const std::unordered_map<std::string,int>& indices);
	void GetJointIndices(std::unordered_map<std::string, int>& m);
	glm::mat4 CreateTransform(const std::vector<float>& matrixValues);
	glm::mat4 GetTransformMatrix(std::string& content);
	Joint* CreateJoint(xmlNode* node);
	std::vector<JointAnimation> ParseAnimation(xmlNode* libraryAnimations, const std::unordered_map<std::string, int>& indices);
	void FreeDocument();
	void MapNameToId(Joint *root, std::unordered_map<std::string, int>& bonesMap, std::unordered_map<std::string, int>& remapIndices);

};



std::string readFile(const char* filePath);


template <typename T>
inline std::vector<T> getArrayData(const std::string& title, const std::string& data)
{
	int start = data.find(title);

	//read the desire row/line
	std::string row{};
	int startpoint = start + title.size();
	while (data[startpoint] != '<')
	{
		row += data[startpoint];
		++startpoint;
	}
	int arrayStart = row.find(">") +1;
	row = row.substr(arrayStart);

	auto splitString = [&](const std::string &txt, std::vector<std::string> &strs, char ch) -> std::size_t
	{
		size_t pos = txt.find(ch);
		size_t initialPos = 0;
		strs.clear();

		// Decompose statement
		while (pos != std::string::npos) {
			strs.push_back(txt.substr(initialPos, pos - initialPos));
			initialPos = pos + 1;
			pos = txt.find(ch, initialPos);
		}
		// Add the last one
		strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

		return strs.size();
	};

	// get the floats in string format from the row
	std::vector<std::string> num_str;
	splitString(row, num_str, ' ');
	if (num_str.back().empty())
		num_str.pop_back();
	
	// cast strings to floats
	std::vector<T> nums;
	for (int i = 0; i < num_str.size(); ++i)
	{
		T f = ::atof(num_str[i].c_str());
		nums.push_back(f);
	}

	return nums;
}


template <typename T>
inline std::vector<T> castStringstoType(const std::vector<std::string>& num_str)
{
	std::vector<T> nums;
	for (int i = 0; i < num_str.size(); ++i)
	{
		T f = ::atof(num_str[i].c_str());
		nums.push_back(f);
	}
	return nums;
}



#endif //COLLADA_PARSER_HPP