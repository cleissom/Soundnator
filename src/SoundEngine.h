#ifndef _SOUNDENGINE
#define _SOUNDENGINE

#include "ofxPDSP.h"
#include "Singleton.hpp"

class SoundEngineBase
{
	pdsp::Engine engine;
public:
	SoundEngineBase() {};
	pdsp::Engine& getEngine() { return engine; };
	pdsp::SequencerSection& getSection(int section) { return engine.sequencer.sections[section]; };
	~SoundEngineBase() {};
};

class SoundEngine : public Singleton<SoundEngineBase>{
private:
	SoundEngine() {};
	~SoundEngine() {};
};

#endif // !_SOUNDENGINE

