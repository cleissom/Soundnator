#ifndef _SOUNDNATOR_APP
#define _SOUNDNATOR_APP


#include "ofMain.h"
#include "TableApp.hpp"
#include "Figure.h"
#include "CollisionHelper.h"
#include "Polygon.h"
#include "ofxPDSP.h"

#include "FigureGraphic.hpp"
#include "InputGestureDirectFingers.hpp"
#include "InputGestureDirectObjects.hpp"
#include "Alarm.hpp"

class Generator;
class Effect;
class Controller;
class Global;
class Output;

static pdsp::Engine engine;

class TableObject : public pdsp::Patchable, public Graphic {
public:
	TableObject(int id = -1) : id(id), dobj(nullptr), followingObj(nullptr), precedingObj(nullptr) {
		registerEvent(InputGestureDirectFingers::I().enterCursor, &TableObject::addCursor, this);
		registerEvent(InputGestureDirectObjects::I().updateObject, &TableObject::updateObject, this);

		addModuleInput("signal", input);
		addModuleInput("pitch", pitch);
		addModuleInput("trig", trig);
		addModuleOutput("signal", output);

		scope.set(bufferLen);
	}

	int getId() {
		return this->id;
	}

	void updateTurnMultiplier(float angle) {
		float derivative = angle - rawAngleLastValue;
		if (derivative > derivativeThreshold) turnsMultiplier--;
		else if (derivative < (-derivativeThreshold)) turnsMultiplier++;
		rawAngleLastValue = angle;
	}

	void addCursor(InputGestureDirectFingers::newCursorArgs & a) {
	}

	void updateObject(InputGestureDirectObjects::updateObjectArgs& a) {
		int id = a.object->f_id;
		if (id == this->id) {
			float rawAngle = a.object->angle;
			updateTurnMultiplier(rawAngle);
			float angle = turnsMultiplier * M_2PI + rawAngle;
			cout << angle << endl;
			updateAngleValue(angle);
		}
	}


	template <typename T>
	bool isObject(TableObject* node) {
		if (dynamic_cast<T*> (node)) {
			return true;
		}
		else {
			return false;
		}
	}

	bool isConnected(TableObject* obj) {
		return obj == followingObj;
	}

	virtual bool canConnectTo(TableObject* obj) {
		if (isObject<Output>(obj)) {
			return true;
		}
		else if ((obj->precedingObj == nullptr) && isConnectableTo(obj)) {
			return true;
		}
		else {
			return false;
		}
	}

	virtual bool isConnectableTo(TableObject* obj) {
		return false;
	}

	void connectTo(TableObject* node) {
		if (followingObj) {
			followingObj->precedingObj = nullptr;
		}
		this->disconnectOut();
		*this >> *node;
		followingObj = node;
		node->precedingObj = this;

	}

	virtual bool haveConnection() {
		return (this->followingObj != nullptr);
	}

	void remove() {
		if (precedingObj && followingObj) {
			precedingObj->disconnectOut();
			followingObj->disconnectIn();
			*precedingObj >> *followingObj;
			precedingObj->followingObj = followingObj;
			followingObj->precedingObj = precedingObj;
			precedingObj = nullptr;
			followingObj = nullptr;
		} else if (followingObj) {
			followingObj->precedingObj = nullptr;
			followingObj = nullptr;
		}
		setDirectObject(nullptr);
		this->disconnectAll();
	}

	void drawAudioWave() {
		ofPushMatrix();

		TableObject* nextObj = getFollowingObject();
		float dist = this->getDistanceTo(getFollowingObject());
		float yHalf = y / 2;
		ofTranslate(this->getDirectObject()->getX(), this->getDirectObject()->getY());
		ofRotateRad(this->getAngleTo(nextObj));
		ofTranslate(0, -yHalf);
		ofNoFill();
		ofSetLineWidth(1);

		int samples = dist / xMult;
		int bufferIndex = 1;
		float yMult = -yHalf;
		vector<float> buffer = scope.getBuffer();

		ofBeginShape();
		for (int xx = 0; xx < samples; xx++) {
			int index = xx * bufferIndex;
			float value = buffer[index];
			ofVertex(xx * xMult, yHalf + value * yMult);
		}
		ofEndShape(false);
		ofPopMatrix();
	}

	void draw() {
		TableObject* nextObj = getFollowingObject();
		if (nextObj) {
			drawAudioWave();
		}

		objectDraw();
	}

	virtual void objectDraw(){}



	virtual void updateAngleValue(float angle) {};


	void setDirectObject(DirectObject* dobj) {
		this->dobj = dobj;
	}

	DirectObject* getDirectObject() {
		return dobj;
	}

	TableObject* getFollowingObject() {
		return followingObj;
	}

	float getDistanceTo(TableObject* obj) {
		return this->dobj->getDistance(obj->dobj);
	}

	float getAngleTo(TableObject* obj) {
		return this->dobj->getAngle(obj->dobj);
	}

	
	void setToScope(pdsp::Patchable& in) {
		in >> scope >> engine.blackhole();
	}

protected:
	pdsp::PatchNode     input;
	pdsp::PatchNode     output;
	pdsp::PatchNode     pitch;
	pdsp::PatchNode     trig;

private:
	int		id;
	float	rawAngleLastValue = 0.0f;
	int		turnsMultiplier = 1;
	const float derivativeThreshold = 1.0f;
	DirectObject* dobj;
	TableObject* followingObj;
	TableObject* precedingObj;

	pdsp::Scope			scope;
	float xMult = 0.0003;
	int bufferLen = 8192;
	float y = 0.05;
};

class Generator : public TableObject {

public:

	Generator(int id = -1) : TableObject(id) {
		patch();
	}
	Generator(const Generator & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void patch() {


		//patching
		osc.out_saw() >> amp >> output;
		this->setToScope(amp);
		
		pitch_ctrl >> osc.in_pitch();
		amp.set(1.0f);
	}	

	void updateAngleValue(float angle) {
		float pitch = ofMap(angle, 0, 5.0f * M_2PI, 36.0f, 96.0f);
		pitch_ctrl.set(pitch);
	}

	bool isConnectableTo(TableObject* obj) {
		return isObject<Effect>(obj) || isObject<Output>(obj);
	}



	void objectDraw() {
		
	}

private:

	pdsp::ValueControl  pitch_ctrl;
	pdsp::Amp           amp;
	pdsp::VAOscillator  osc;
	
};

class Effect : public TableObject {

public:

	Effect(int id = -1) : TableObject(id) {
		patch();
	}
	Effect(const Effect  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile


	void patch() {

		input >> filter >> amp >> output;
		this->setToScope(amp);
		cutoff_ctrl >> filter.in_cutoff();
		amp.set(1.0f);
	}

	void addCursor(InputGestureDirectFingers::newCursorArgs & a) {
	}

	void updateAngleValue(float angle) {
		float cutoff = ofMap(angle, 0, 2.0f * M_2PI, 48.0f, 96.0f);
		cutoff_ctrl.set(cutoff);
	}

	bool isConnectableTo(TableObject* obj) {
		return isObject<Effect>(obj) || isObject<Output>(obj);
	}


private:
	pdsp::Amp           amp;
	pdsp::ValueControl	cutoff_ctrl;
	pdsp::VAFilter      filter; // 24dB multimode filter

};

class Output : public TableObject {

public:

	Output(int id = -1) : TableObject(id) {

		patch();
	}
	Output(const Output  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile


	void patch() {

		//------------SETUPS AND START AUDIO-------------
#ifdef PC
		engine.setDeviceID(1);
#else
		engine.setDeviceID(0);
#endif // PC

		engine.setup(44100, 512, 3);

		input >> engine.audio_out(0);
		input >> engine.audio_out(1);
	}

	void draw() {
		ofFill();
		ofDrawCircle(getDirectObject()->getX(), getDirectObject()->getY(), 0.03f);
	}

	bool canConnectTo(TableObject* obj) {
		return false;
	}

	bool haveConnection() {
		return true;
	}

private:
};

class SoundnatorApp : public ofBaseApp{

	public:

        TableApp tableapp;
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

		// pdsp modules
};

#endif
