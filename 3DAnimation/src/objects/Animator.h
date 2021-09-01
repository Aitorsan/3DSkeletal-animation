#pragma once
#include <vector>
#include <algorithm>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "../app/Joint.h"
#include "../app/JointAnimation.h"

class Animator
{
	Joint* Root;
	float Duration; 
	float CurrentTime;
	bool ApplyCorrection;
	std::vector<JointAnimation> Animation;
	std::vector<glm::mat4> CurrentPosTransform;
public:
	KeyFrame Previous;
	KeyFrame Next;
	Animator( Joint* root, const std::vector<JointAnimation>& animation,bool correction = false)
		: Root{root}
		, Duration{}
		, CurrentTime{}
		, ApplyCorrection{correction}
		, Animation{animation}
		, CurrentPosTransform(animation.size(),glm::mat4(1.0f))
    {


		Duration = ComputeDuration(Animation);

	}

	Animator(Animator& anim)
		: Root{ nullptr }
		, Duration{}
		, CurrentTime{}
		, ApplyCorrection{ anim.ApplyCorrection }
		, Animation{ anim.Animation }
		, CurrentPosTransform(Animation.size(), glm::mat4(1.0f))
	{
		//CopyTree(Root, anim.Root);
	}

	~Animator()
	{
		if (Root)
		{
			delete Root;
			Root = nullptr;
		}
	}

	float ComputeDuration(const std::vector<JointAnimation>& anim)
	{
		float duration = 0;
		for (const JointAnimation& joint : anim)
		{
			if (!joint.Frames.empty())
			{
				for (KeyFrame f : joint.Frames)
				{
					if (f.TimeStamp >= duration)
					{
						duration = f.TimeStamp;
					}
				}
			}

		}
		return duration;
	}



	void Update(float dt)
	{
		  CurrentTime += 1* dt;
		  CurrentTime = fmod(CurrentTime, Duration);
		  CalculateBoneTransform(Root);
	}
	


	void CalculateBoneTransform(Joint* node)
	{
		auto it = std::find_if(Animation.begin(), Animation.end(), [&](auto& a) {return a.jointName == node->Channel;});

		
		assert(it != Animation.end() && "Bone should be always there\n");

		glm::mat4 localNewPositionTransform = node->localBindTransform;

		// if this bone has animation compute the local transform for this bone
		if (!it->Frames.empty())
		{
			localNewPositionTransform = ComputeNewBonePosition(it->Frames);			
		}

		// add the transform from parent to child
		CurrentPosTransform[node->ID] =  localNewPositionTransform;


		// repite process with all the children
		for (Joint* child : node->Children)
		{
			CalculateBoneTransform(child);
		}
	}


	glm::mat4 ComputeNewBonePosition(const std::vector<KeyFrame>& frames)
	{
		//Find previous key frame and next keframe for the Current time
		KeyFrame previous{Previous},next{Next};
		bool found = false;
		for (const KeyFrame& kframe : frames)
		{
			if (kframe.TimeStamp <= CurrentTime)
			{
				found = true;
				previous = kframe;
			}
			else if (kframe.TimeStamp > CurrentTime)
			{
				next = kframe;
				break;
			}
		}

		Previous = previous; Next = next;

		// Get the rotation and translation in vector and quaternion form from previos and next frame
		glm::quat prevRot{}; glm::vec3 prevTrans{};
	    ExtractRotationAndTranslation(prevRot,prevTrans,previous.Transform);

		glm::quat nextRot{}; glm::vec3 nextTrans{};
		ExtractRotationAndTranslation(nextRot, nextTrans, next.Transform);

		// Get the progression factor to interpolate base on it
		float progression = GetProgression(previous.TimeStamp, next.TimeStamp,CurrentTime);

		// Interpolate 
		glm::quat finalRotation = InterpolateRotations(prevRot, nextRot, progression);
		glm::vec3 finalTranslation = InterpolateTranslations(prevTrans, nextTrans,progression);
		// final transforms
		glm::mat4 finalBoneLocalPositionTransform = GetMatrixForm(finalRotation, finalTranslation);

		return finalBoneLocalPositionTransform;
	}
	
	void ExtractRotationAndTranslation(glm::quat& rot, glm::vec3& offset, glm::mat4 transform)
	{
		glm::vec3 scale;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform, scale, rot, offset, skew, perspective);
	}

	float GetProgression(float previousTimeStamp, float nextTimeStamp, float currentTime)
	{
		float totalTime = nextTimeStamp - previousTimeStamp;
		float currentTimePoint = currentTime - previousTimeStamp;
		return currentTimePoint / totalTime;
	}



	glm::quat InterpolateRotations(glm::quat& prev, glm::quat& next, float progression)
	{
		glm::quat rotation = glm::slerp(prev,next,progression);
		rotation = glm::normalize(rotation);
		return rotation;
	}

	glm::vec3 InterpolateTranslations(glm::vec3& prev, glm::vec3& next, float progression)
	{

		return glm::mix(prev, next, progression);
	}

	glm::mat4 GetMatrixForm(glm::quat& rotation, glm::vec3& offset)
	{
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), offset);
		glm::mat4 rotationMatrix = glm::toMat4(rotation);
		return   translationMatrix * rotationMatrix;
	}



	std::vector<glm::mat4> GetBoneTransforms()
	{
		return CurrentPosTransform;
	}
};