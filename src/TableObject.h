#ifndef _TABLEOBJECT
#define _TABLEOBJECT

#include "ofMain.h"
#include "TableApp.hpp"
#include "Figure.h"
#include "CollisionHelper.h"
#include "Polygon.h"
#include "ofxPDSP.h"

#include "FigureGraphic.hpp"
#include "InputGestureDirectFingers.hpp"
#include "InputGestureDirectObjects.hpp"
#include "InputGestureTap.hpp"

#include "SoundEngine.h"
#include "TableUI.h"

class Generator;
class Effect;
class Controller;
class Global;
class Output;


typedef enum { AUDIO, CONTROL } connectionType_t;


class TableObject : public pdsp::Patchable, public Graphic {
public:
	TableObject(int id = -1, connectionType_t connection = AUDIO);

	int getId();

	void updateTurnMultiplier(float angle);

	void updateObject(InputGestureDirectObjects::updateObjectArgs& a);

	template <typename T>
	bool isObject(TableObject* node);


	TableObject** getPrecedingObj(TableObject* obj);
	TableObject** getPrecedingObj(TableObject* obj, connectionType_t connection);

	void makeConnectionTo(TableObject* obj);
	void makeDisconnectionOut(TableObject* obj);
	void makeDisconnectionIn(TableObject* obj);



	bool isConnectedTo(TableObject* obj);
	virtual bool canConnectTo(TableObject* obj);
	virtual bool objectIsConnectableTo(TableObject* obj) = 0;
	virtual bool objectIsConnectableToOutput() = 0;
	void connectTo(TableObject* obj);
	virtual bool haveConnection();
	void remove();

	void drawAudioWave();
	void draw();
	virtual void objectDraw() {}

	virtual void updateAngleValue(float angle) {};

	void setDirectObject(DirectObject* dobj);
	DirectObject* getDirectObject();
	TableObject* getFollowingObject();
	float getDistanceTo(TableObject* obj);
	float getAngleTo(TableObject* obj);
	void setToScope(pdsp::Patchable& in);

	void updateTableUI(TableUIBase * ui, bool conditional = true);

	void loadImg(ofImage & image, const std::string & dir);

protected:
	pdsp::PatchNode     input;
	pdsp::PatchNode     output;
	pdsp::PatchNode     pitch_in;
	pdsp::PatchNode     pitch_out;
	pdsp::PatchNode     trig_in;
	pdsp::PatchNode     trig_out;
	bool connectionUpdated;

	float angleMaxValue = TWO_PI;
	float angleMinValue = -TWO_PI;

private:
	int		id;
	float	rawAngleLastValue = 0.0f;
	int		turnsMultiplier = 0;
	const float derivativeThreshold = 2.0f;
	connectionType_t connection;
	DirectObject* dobj;
	TableObject* followingObj;
	TableObject* precedingAudioObj;
	TableObject* precedingControlObj;

	pdsp::Scope			scope;
	float xMult = 0.0003;
	int bufferLen = 8192;
	float y = 0.05;

};


class Generator : public TableObject {

public:

	Generator(int id = -1, connectionType_t connection = AUDIO);

	bool objectIsConnectableTo(TableObject* obj);
	bool objectIsConnectableToOutput();

private:

};


class Oscillator : public Generator {

public:
	typedef enum {SINE, SAW, PULSE} oscillatorMode;

	Oscillator(int id = -1);
	Oscillator(const Generator & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void update();
	void patch();
	void updateAngleValue(float angle);
	void updateVolume(TableSlider::updateSliderArgs & a);
	void Tap(TableButton::TapButtonArgs & a);

private:
	oscillatorMode actualMode;
	bool actualModeChanged;
	TableButton*  button;
	TableSlider*  slider;
	pdsp::ValueControl  pitch_ctrl;
	pdsp::Amp           ampEnv;
	pdsp::Amp           amp;
	pdsp::VAOscillator  sine;
	pdsp::VAOscillator  saw;
	pdsp::VAOscillator  pulse;
	pdsp::ADSR			env;

	ofImage sineImg;
	ofImage sawImg;
	ofImage pulseImg;
};


class Sampler : public Generator {

public:
	typedef enum { SINE, SAW, PULSE } oscillatorMode;

	Sampler(int id = -1);
	Sampler(const Sampler & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void update();
	void patch();
	void updateAngleValue(float angle);
	void updateVolume(TableSlider::updateSliderArgs & a);
	void Tap(TableButton::TapButtonArgs & a);

private:
	oscillatorMode actualMode;
	int choose = 0;
	TableButton*  button;
	TableSlider*  slider;
	TableInfoCircle*  info;
	pdsp::ValueControl  pitch_ctrl;
	pdsp::Amp           ampEnv;
	pdsp::Amp           amp;
	pdsp::VAOscillator  sine;
	pdsp::VAOscillator  saw;
	pdsp::VAOscillator  pulse;
	pdsp::AHR			env;

	ofImage sineImg;
	ofImage sawImg;
	ofImage pulseImg;

	pdsp::SampleBuffer violin;
	pdsp::Sampler sampler;
};

class Effect : public TableObject {

public:

	Effect(int id = -1, connectionType_t connection = AUDIO);
	bool objectIsConnectableTo(TableObject* obj);
	bool objectIsConnectableToOutput();


private:
};

class Filter : public Effect {
public:
	typedef enum { LOWPASS, HIGHPASS, BANDPASS } filterMode;

	Filter(int id = -1);
	Filter(const Filter  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile
	void patch();

	void update();
	void updateAngleValue(float angle);
	void Tap(TableButton::TapButtonArgs & a);
	void updateSlider(TableSlider::updateSliderArgs & a);

private:
	filterMode actualMode = LOWPASS;
	bool actualModeChanged = false;

	const float filterMinValue = 36.0;
	const float filterMaxValue = 130.0;

	TableInfoCircle* info;
	TableButton* button;
	TableSlider* slider;

	pdsp::Amp           ampDry;
	pdsp::Amp           ampWet;
	pdsp::Amp           amp;
	pdsp::ValueControl	cutoff_ctrl;
	pdsp::VAFilter      filter; // 24dB multimode filter
};

class Delay : public Effect {
public:
	typedef enum { FEEDBACK_DELAY, REVERB } delayMode;

	Delay(int id = -1);
	Delay(const Delay  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile
	void patch();

	void update();
	void updateAngleValue(float angle);
	void Tap(TableButton::TapButtonArgs & a);
	void updateSlider(TableSlider::updateSliderArgs & a);

private:
	delayMode actualMode = FEEDBACK_DELAY;
	bool actualModeChanged = false;

	const float delayMinValue = 0.0f;
	const float delayMaxValue = 200.0f;

	TableInfoCircle* info;
	TableButton* button;
	TableSlider* slider;

	pdsp::Amp           amp;
	pdsp::ValueControl	time_ctrl;
	pdsp::ValueControl	feedback_ctrl;
	pdsp::Delay			delay;
	pdsp::BasiVerb		reverb;
	pdsp::Switch		switcher;
	pdsp::PatchNode		node;
};


class Controller : public TableObject {

public:

	Controller(int id = -1, connectionType_t connection = CONTROL);

	bool objectIsConnectableTo(TableObject* obj);
	bool objectIsConnectableToOutput();

private:

};

class Sequencer : public Controller {

public:
	typedef enum { SEQUENCER, PITCH, VOLUME } sequencerMode;

	Sequencer(int id = -1, int sequencerSection = 0, connectionType_t connection = CONTROL);
	Sequencer(const Sequencer  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void patch();

	void update();
	void updateAngleValue(float angle);

	void updateTableSequencerCells(TableSequencerCells::updateTableSequencerCellsArgs & a);
	void updateTableSequencerPitch(TableSequencerSliders::updateTableSequencerSlidersArgs & a);
	void updateTableSequencerVolume(TableSequencerSliders::updateTableSequencerSlidersArgs & a);
	void tapButton(TableButton::TapButtonArgs & a);
	void longPushButton(TableButton::LongPushButtonArgs & a);
	void updateTempoSlider(TableSlider::updateSliderArgs & a);


private:
	pdsp::Amp           amp;
	pdsp::VAOscillator  osc;
	pdsp::ValueControl	pitch_ctrl;
	pdsp::VAFilter      filter; // 24dB multimode filter

	vector< vector<bool> > beats;
	vector< vector<int> > pitches;
	vector< vector<int> > volumes;

	int beatsNum = 16;

	sequencerMode actualMode;

	bool showSlider = false;
	bool showTableSequencerCells = false;
	bool showTableSequencerPitch = false;
	bool showTableSequencerVolume = false;
	int actualSequence;

	int sequencerSection;


	TableSequencerCells*	tableSequencerCells;
	TableSequencerSliders*	tableSequencerPitch;
	TableSequencerSliders*	tableSequencerVolume;
	TableButton*			button;
	TableSlider*			tempoSlider;
	TableInfoCircle*		info;
};


class Output : public TableObject {

public:

	Output(int id = -1);
	Output(const Output  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void patch();
	void draw();
	bool haveConnection();
	bool objectIsConnectableTo(TableObject* obj);
	bool objectIsConnectableToOutput();

private:
};

#endif // !_TABLEOBJECT

