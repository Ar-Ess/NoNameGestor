#ifndef SOLOUD_TYPES_H
#define SOLOUD_TYPES_H

// Minimal core definitions moved out of soloud.h to break include cycles.
// Only define what's required by the other component headers.

#ifndef SOLOUD_CORE_DEFINED
#define SOLOUD_CORE_DEFINED

// Configuration defines used by headers
#define FILTERS_PER_STREAM 8
#define SAMPLE_GRANULARITY 512
#define VOICE_COUNT 1024
#define MAX_CHANNELS 8

namespace SoLoud
{
	typedef unsigned int result;
	typedef unsigned int handle;
	typedef double time;
	// Forward declarations used across headers
	class Soloud;
	class Fader;
	class AlignedFloatBuffer;
	class Filter;
	class FilterInstance;
}

#endif // SOLOUD_CORE_DEFINED

#endif // SOLOUD_TYPES_H
