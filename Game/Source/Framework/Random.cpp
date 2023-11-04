#include "Random.h"
#include "Maths.h"

Random& Random::Instance()
{
	static Random instance;
	return instance;
}

float Random::RealFloat(float min, float max)
{
	return Instance().Float(true, min, max);
}

float Random::PsdoFloat(float min, float max)
{
	return Instance().Float(false, min, max);
}

int Random::RealInt(int min, int max)
{
	return Instance().Int(true, min, max);
}

int Random::PsdoInt(int min, int max)
{
	return Instance().Int(false, min, max);
}

bool Random::RealBool()
{
	return (bool)Instance().Int(true, 0, 1);
}

bool Random::PsdoBool()
{
	return (bool)Instance().Int(false, 0, 1);
}

void Random::Seed(int seed)
{
	if (seed == 0) seed = 1;

	Instance().psdoSeed = seed;
	Instance().realSeed = seed;
}

float Random::Float(bool real, float min, float max)
{
	float ret = 0;

	if (real)
	{
		uint32_t* ptr = new uint32_t(realSeed);
		intptr_t cast = reinterpret_cast<intptr_t>(ptr);
		delete ptr;
		uintptr_t seed = (cast % 1000000000);

		uint32_t state = realSeed * seed * 747796405u * 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		realSeed = (word >> 22u) ^ word;
		ret = Maths::Abs(realSeed) / (float)UINT32_MAX;
	}
	else
	{
		uint32_t state = psdoSeed * 747796405u * 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		psdoSeed = (word >> 22u) ^ word;
		ret = Maths::Abs(psdoSeed) / (float)UINT32_MAX;
	}

	return ((max - min) * ret) + min;
}

float Random::Int(bool real, int min, int max)
{
	return Maths::Aproximate(Float(real, min, max), 0.5f);
}
