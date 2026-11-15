#pragma once

namespace SoLoud {
	class Soloud;
	class EchoFilter;
	class FreeverbFilter;
}
class Audio;
class Spatialization;

class Mixer
{

	friend class AudioSystem;

public:

	~Mixer();

private:

	Mixer(SoLoud::Soloud* engine);

	// Resolve the audio's volume and panning depending on this mixer
	// If "calcPan" set false, allows to quit calculating the panning if not needed
	void ResolveMixerState(float& volume, float& pan, const bool calcPan);

	void RealParams(const Mixer* source, float& volume, float& pan, const bool calcPan) const;

	float InternalVolume() const;
	float InternalVolume(float value) const;

	float InternalPan() const;
	float InternalPan(float value) const;

private:

	Mixer* parent = nullptr;
	SoLoud::Soloud* engine = nullptr;
	//SoLoud::EchoFilter* echo = nullptr;
	//SoLoud::FreeverbFilter* reverb = nullptr;
	
	float volume = 1;
	float pan = 0;

};
