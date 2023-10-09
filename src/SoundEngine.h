#ifndef _SOUNDENGINE
#define _SOUNDENGINE

#include "Singleton.hpp"
#include "ofxPDSP.h"

class SoundEngineBase {
    pdsp::Engine engine;

   public:
    SoundEngineBase(){};
    pdsp::Engine &getEngine() { return engine; };
    pdsp::SequencerSection &getSection(int section) { return engine.sequencer.sections[section]; };
    ~SoundEngineBase(){};
};

class SoundEngine : public Singleton<SoundEngineBase> {
   private:
    SoundEngine(){};
    ~SoundEngine(){};
};

#endif  // !_SOUNDENGINE
