#include "ColladaParser.h"
#include <queue>
#include <utility>
#include <iomanip>
#include <cassert>
#include <regex>
#include "app/Joint.h"
#include "app/JointAnimation.h"

#define READ_COLLADA_ERR  "can't read collada file"
#define VISUAL_SCENES "library_visual_scenes"
#define CONTROLLERS "library_controllers"
#define ANIMATIONS "library_animations"

void print_tree(Joint* root)
{
	std::vector< Joint*> stack{root  };
	while (!stack.empty())
	{
		Joint* cur = stack.front();
		stack.erase(stack.begin());

		std::cout <<cur->Name << ":" << cur->ID << std::endl;
		std::vector<Joint*> order;

		for ( Joint* child : cur->Children)
			order.insert(order.begin(),child);
	
		for (auto& pair : order)
			stack.insert(stack.begin(), pair);
	}
}

std::string readFile(const char* filePath)
{
	std::vector<char>buffer;
	std::fstream f(filePath, std::ios::in | std::ios::ate);

	if (!f.is_open()) { assert(0 && "failed open file"); }

	int size = f.tellg();
	buffer.resize(size);
	f.seekg(0);
	f.read(buffer.data(), size);
	f.close();

	std::string data{ buffer.begin(),buffer.end() };

	return data;
}


glm::mat4 ColladaParser::CreateTransform(const std::vector<float>& matrixValues)
{
	constexpr int stride = 16;
	int matsize = matrixValues.size();

	glm::mat4 transform{};
	for (int i = 0; i < matsize; i += stride)
	{
		glm::mat4 m{};
		int start = i;

		for (int c = 0; c < 4; ++c)
		{
			for (int r = 0; r < 4; ++r)
			{
				m[c][r] = matrixValues[start++];
			}
		}

		// apply z correction of 90 degrees around x axis. Blender has Z axis Up
		// if we set Y up in blender and -Z forward so we don't need this line
		m = glm::transpose(m);
		if (ApplyAxisCorrection)
		{
			m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1, 0, 0)); 
			// only apply correction to root transform
			ApplyAxisCorrection = false;
		}
		transform = m;
	}
	return transform;
}

ColladaParser::ColladaParser()
	: XmlDocument{nullptr}
	, Root{nullptr}
	, LibraryVisualScenes{nullptr}
	, LibraryControllers{nullptr}
	, Armature{nullptr}
	, RootJoint{nullptr}
	, Animation{}
	, ApplyAxisCorrection{false}
{
	LIBXML_TEST_VERSION
}

ColladaParser::ColladaParser(ColladaParser && other)
	: XmlDocument{ other.XmlDocument }
    , Root { other.Root}
	, LibraryVisualScenes{ other.LibraryVisualScenes }
	, LibraryControllers{ other.LibraryControllers }
	, Armature{ other.Armature }
	, RootJoint{ other.RootJoint }
	,  Animation{ other.Animation}
	, ApplyAxisCorrection{other.ApplyAxisCorrection}
{
	other.XmlDocument = nullptr;
	other.Root = nullptr;
	other.LibraryVisualScenes = nullptr;
	other.LibraryControllers = nullptr;
	other.Armature = nullptr;
	other.RootJoint = nullptr;
}

ColladaParser & ColladaParser::operator=(ColladaParser && other)
{
	if (this == &other) return *this;
	 
	XmlDocument = other.XmlDocument;
	Root = other.Root;
	LibraryVisualScenes = other.LibraryVisualScenes;
	LibraryControllers = other.LibraryControllers;
	Armature = other.Armature;
	RootJoint = other.RootJoint;
	ApplyAxisCorrection = other.ApplyAxisCorrection;
	Animation = std::move(other.Animation);

	other.XmlDocument = nullptr;
	other.Root = nullptr;
	other.LibraryVisualScenes = nullptr;
	other.LibraryControllers = nullptr;
	other.Armature = nullptr;
	other.RootJoint = nullptr;

	return *this;
}

inline std::size_t splitString(const std::string &txt, std::vector<std::string> &strs, char ch) 
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

void ColladaParser::FreeDocument()
{
	xmlFreeDoc(XmlDocument);
}

ColladaParser::~ColladaParser()
{
	if (XmlDocument)
		FreeDocument();
	xmlCleanupParser();
	xmlMemoryDump();
}

std::vector<xmlNode*> findAllWithProperty(xmlNode* a_node, const char* propertyName, const char* value)
{
	std::vector<xmlNode*> nodes;
	std::queue<xmlNode*> q;
	q.push(a_node);

	while (!q.empty())
	{
		xmlNode *cur_node = nullptr;
		cur_node = q.front();
		q.pop();
		if (cur_node)
		{
			xmlAttr* attr = cur_node->properties;
			
			while(attr)
			{
				if ( strcmp((const char*)attr->name, propertyName) == 0 
					&& attr->children 
					&& strcmp((const char*)attr->children->content, value) == 0)
				{

					nodes.push_back(cur_node);
				}
				attr = attr->next;
			}

			xmlNode* child = cur_node->children;

			while (child != nullptr)
			{
					q.push(child);
					child = child->next;
			}
		}
	}
	return nodes;
}



xmlNode* findByPorperty(xmlNode* a_node, const char* propertyName, std::regex reg)
{
	std::queue<xmlNode*> q;
	q.push(a_node);
	xmlNode* foundNonde = nullptr;

	while (!q.empty())
	{
		xmlNode *cur_node = nullptr;
		cur_node = q.front();
		q.pop();
		if (cur_node)
		{
			xmlAttr* attr = cur_node->properties;

			while (attr)
			{
				if (strcmp((const char*)attr->name, propertyName) == 0 && attr->children )
				{
					std::string name = (const char*)attr->children->content;					

					if (std::regex_match(name, reg))
					{
						std::cout << name << std::endl;
						foundNonde = cur_node;
						cur_node = nullptr;
						break;
					}				
				}
				attr = attr->next;
			}
			if (foundNonde) break;

			xmlNode* child = cur_node->children;

			while (child != nullptr)
			{
				if (cur_node->type == XML_ELEMENT_NODE)
				{
					q.push(child);
					child = child->next;
				}
			}
		}
	}
	return foundNonde;
}

xmlNode* findNodeByName(xmlNode * a_node, const char* name)
{
	std::queue<xmlNode*> q;
	q.push(a_node);
	xmlNode* foundNonde = nullptr;

	while (!q.empty())
	{
		xmlNode *cur_node = nullptr;
		cur_node = q.front();
		q.pop();
		if (cur_node && cur_node->type == XML_ELEMENT_NODE)
		{
			if (std::strcmp((const char*)cur_node->name, name) == 0)
			{
				foundNonde = cur_node;
				cur_node = nullptr;
				break;
			}

			xmlNode* child = cur_node->children;

			while (child != nullptr)
			{
				if (cur_node->type == XML_ELEMENT_NODE)
				{
					q.push(child);
					child = child->next;
				}
			}
		}
	}
	return foundNonde;
}


Joint* ColladaParser::GetJointHerarchy(const char * path,bool applyAxisCorrection)
{
	ApplyAxisCorrection = applyAxisCorrection;

	xmlKeepBlanksDefault(0);
	XmlDocument = xmlReadFile(path, NULL, 0);
	if (XmlDocument == NULL)
		assert(0 && READ_COLLADA_ERR && path);

	Root = xmlDocGetRootElement(XmlDocument);
	LibraryControllers = findNodeByName(Root, CONTROLLERS);
	LibraryVisualScenes = findNodeByName(Root, VISUAL_SCENES);
    Armature = findByPorperty(LibraryVisualScenes, "id", std::regex("(Armature)"));
	RootJoint = findByPorperty(Armature, "type", std::regex("(JOINT)"));
	std::unordered_map<std::string, int> boneIndices{};
	GetJointIndices(boneIndices);
    SkeletonRoot = ParseSkeleton(RootJoint,nullptr,boneIndices);
	SkeletonRoot->CalculateInverseBindTransform(glm::mat4(1.0f));
	//print_tree(SkeletonRoot);
	return SkeletonRoot;
}

std::vector<JointAnimation> ColladaParser::GetAnimation(const char* filepath)
{
	xmlKeepBlanksDefault(0);
	xmlDocPtr document = xmlReadFile(filepath, NULL, 0);

	if (document == NULL)
		assert(0 && READ_COLLADA_ERR && filepath);

	xmlNode* root = xmlDocGetRootElement(document);
	xmlNode* libraryAnimations = findNodeByName(root, ANIMATIONS);
	
	if (!libraryAnimations) return {};

	std::unordered_map<std::string, int> boneIndices{};
	GetJointIndices(boneIndices);

	std::unordered_map<std::string, int> remapIndices;
	//map name to id
	MapNameToId(SkeletonRoot, boneIndices, remapIndices);

	Animation = ParseAnimation(libraryAnimations, remapIndices);

	xmlFreeDoc(document);
	xmlCleanupParser();
	xmlMemoryDump();

	return Animation;
}


void ColladaParser::MapNameToId(Joint *root, std::unordered_map<std::string, int>& bonesMap, std::unordered_map<std::string, int>& remapIndices)
{
	std::string channel = root->Channel;

	int id = bonesMap.at(root->Name);
	remapIndices[channel] = id;

	for (Joint* child : root->Children)
	{
		MapNameToId(child, bonesMap, remapIndices);
	}

}

glm::mat4 ColladaParser::GetTransformMatrix(std::string& content)
{
	std::vector<std::string> values_str{};
	splitString(content, values_str, ' ');
	if (values_str.back().empty())
		values_str.pop_back();

	std::vector<float> values = std::move(castStringstoType<float>(values_str));
	glm::mat4 t = CreateTransform(values);
	return t;
}

Joint* ColladaParser::CreateJoint(xmlNode* node)
{
	Joint* j = new Joint;
	j->Name = (const char*)node->properties->next->next->children->content;
	j->Channel = (const char*)node->properties->children->content;

	xmlNode* transform = findByPorperty(node, "sid", std::regex("(transform)"));

	// It must have a transform, if not something went terrible wrong
	assert(transform != nullptr);

	if (transform && transform->children)
	{
		std::string content = (const char*)transform->children->content;
		assert(!content.empty());
		glm::mat4 t = GetTransformMatrix(content);
		j->localBindTransform = t;
	}

	return  j;
}

std::vector<JointAnimation> ColladaParser::ParseAnimation(xmlNode* libraryAnimations, const std::unordered_map<std::string, int>& boneIndices)
{

	// copy the bones to a vector of Joint animations for futher processing
	std::vector<JointAnimation> jointAnimations(boneIndices.size());
	for (auto& j : boneIndices)
		jointAnimations[j.second].jointName = j.first;

	std::vector<xmlNode*> jointAnimNodes;

	// In blender you can assing your animations inside actions. If there exist an action container
	// then we continue if there is not action container then we need to skip some code to make this work
	std::string actionContainerStr = (const char*)xmlGetProp(libraryAnimations->children, (xmlChar*)"id");
	
	// check if the first child is an action container node
	int count = {};
	if (actionContainerStr.find("action") == std::string::npos)
	{
		// there is no action container so we set the libraryAnimations root the one to check the children
		count = xmlChildElementCount(libraryAnimations);

		jointAnimNodes.push_back(libraryAnimations->children);
		// advance to the first children
		libraryAnimations = libraryAnimations->children;
		//Find all the bones that has an animation node with their correspoding transform
		for (int i = 1; i < count; ++i)
		{
			// for every bone that has an animation transform
			xmlNode* animatedBone = libraryAnimations->next;
			jointAnimNodes.push_back(animatedBone);
			libraryAnimations = libraryAnimations->next;
		}
	}
	else
	{
		// we assume there is only one animation container that contains multiple animation nodes for each bone
		count = xmlChildElementCount(libraryAnimations->children);
		//advance to the animation node
		libraryAnimations = libraryAnimations->children;

		jointAnimNodes.push_back(libraryAnimations->children);
		libraryAnimations = libraryAnimations->children;
		//Find all the bones that has an animation node with their correspoding transform
		for (int i = 1; i < count; ++i)
		{
			// for every bone that has an animation transform
			xmlNode* animatedBone = libraryAnimations->next;
			jointAnimNodes.push_back(animatedBone);
			libraryAnimations = libraryAnimations->next;
		}
	}

	// Animations
	for (xmlNode* node: jointAnimNodes)
	{
		// the last child is the channel
		xmlNode* channel = xmlGetLastChild(node);
		// get the content of the target propertie this will tell use which bone is targeting
		std::string targetBone =(const char*) xmlGetProp(channel, (xmlChar*)"target");

		// find input data it should be the first node always
		xmlNode* inputNode = node->children;
		// get the keyframes
		std::string keyframeData = (const char*) xmlNodeGetContent(inputNode);
		//find the output pose matrix   should be second sibling of the input Node
		xmlNode* outputNode = inputNode->next;
		// get the matrices
		std::string matrixTransformsData = (const char*)xmlNodeGetContent(outputNode);
	    //split the string -> cast to float -> return an array
		//key frames
		std::vector<std::string> strings{};
		splitString(keyframeData, strings,' ');
		std::vector<float> keyframes = castStringstoType<float>(strings);
		//matrices
		strings.clear();
		splitString(matrixTransformsData, strings, ' ');
		std::vector<float> transformNumbers = castStringstoType<float>(strings);

		std::vector<glm::mat4> transforms;
		transforms.reserve(transformNumbers.size() / 16);
		
		for (int startingIndex = 0; startingIndex < transformNumbers.size(); startingIndex+=16)
		{
			
			glm::mat4 matrix = CreateTransform({ transformNumbers.begin() + startingIndex,transformNumbers.begin() + startingIndex + 16 });
			transforms.push_back(matrix);

		}

		std::vector<KeyFrame> keyframesVector;
		keyframesVector.reserve(keyframes.size());
		for (int i = 0; i < transforms.size(); ++i)
		{
			KeyFrame k;
			k.TimeStamp = keyframes[i];
			k.Transform = transforms[i];
			keyframesVector.push_back(k);
		}	
		std::vector<std::string> targetBoneStrings;
		splitString(targetBone, targetBoneStrings, '/');
		std::string boneName = targetBoneStrings.front();

		auto it = std::find_if(jointAnimations.begin(), jointAnimations.end(), [&](const JointAnimation& first) {
			return first.jointName == boneName;
			});

	
		// if we do not find it then check the names in the collada file 
		if (it != jointAnimations.end())
		{
			it->Frames = std::move(keyframesVector);
		}
	}
	return jointAnimations;
}

Joint* ColladaParser::ParseSkeleton(xmlNode* node , Joint* parent,const std::unordered_map<std::string,int>& indices)
{
	if (node && node->properties && node->properties->children)
	{
		Joint* j = CreateJoint(node);
		assert(j);

		if (indices.find(j->Name) == indices.end()) return nullptr;//TODO: handle this situation
		j->ID = indices.at(j->Name);
		
		if (parent)
			parent->Children.push_back(j);
		else
			parent = j;
		//process all childs
		xmlNode* next = node->children;
		while (next)
		{
			xmlAttr* attr = next->properties;

			while (attr)
			{
				if (strcmp((const char*)attr->name, "type") == 0
					&& attr->children
					&& strcmp((const char*)attr->children->content, "JOINT") == 0)
				{
					ParseSkeleton(next, j,indices);
				}
				attr = attr->next;
			}
			next = next->next;
		}
	}
	return  parent;
}

void ColladaParser::GetJointIndices(std::unordered_map<std::string,int>& m)
{
	xmlNode* Ids = findByPorperty(LibraryControllers, "id",std::regex( "Armature\_[[:w:]]+\-skin\-joints\-array"));

	assert(Ids && "Error parsing Joint Indices");
	assert(Ids->last &&"Error parsin Joint Indices has no content");

	std::string s = (const char*) Ids->last->content;

	std::vector<std::string> values_str{};
	splitString(s, values_str, ' ');

	for (int i = 0; i< values_str.size();++i)
		m[values_str[i]] = i;
}
