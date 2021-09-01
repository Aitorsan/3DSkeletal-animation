#pragma once
#include <random> // for std::mt19937
#include <ctime> // for std::time

namespace MyRandom
{
	// Initialize our mersenne twister with a random seed based on the clock (once at system startup)
	std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
}

int getRandomNumber(int min, int max)
{
	std::uniform_int_distribution<> die{ min, max }; // we can create a distribution in any function that needs it
	return die(MyRandom::mersenne); // and then generate a random number from our global generator
}




std::vector<float> perlinNoise1D( int octaves)
{

	constexpr int seedSize = 20;
	
	float seedVals[seedSize];
	for (int i = 0; i < seedSize; ++i)
	{
		seedVals[i] = (float)rand() / (float)RAND_MAX;
	}

	
	std::vector<float> output;

	for (int x = 0; x < seedSize; ++x)
	{
		float noise = 0.0f;
		float scaleAccumulate = 0.0f;
		float scaleFactor = 1.0f;

		for (int o = 0; o < octaves; ++o)
		{
			int pitch = (seedSize >> o)+1; // divide by 2 depending on the number of actives
			int sample1 = (x / pitch) * pitch;
			int sample2 = (sample1 + pitch) % pitch;

			float blend = (float)(x - sample1);// / (float)pitch;// get a value between 0 and 1
		    
			// linearly interpolate the 2 sample values
			float sample = (1.0f - blend) * seedVals[sample1] + blend * seedVals[sample2];
		  
			//add the scale factor to divide
			scaleAccumulate += scaleFactor;
			// scale acordingly
			noise += sample * scaleFactor;
			// now we need to half the scale value
			scaleFactor = scaleFactor / 1.4f;
		}
		//add the noise
		output.push_back(noise);

	}

	return output;

}
