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

class TableObject : public pdsp::Patchable, public Graphic {
public:
	TableObject(int id = -1) : id(id), dobj(nullptr), followingObj(nullptr) {
		registerEvent(InputGestureDirectFingers::I().enterCursor, &TableObject::addCursor, this);
		registerEvent(InputGestureDirectObjects::I().updateObject, &TableObject::updateObject, this);
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
	void connectToIfType(TableObject* node) {
		if (auto cast_node = dynamic_cast<T*> (node)) {
			*this >> *cast_node;
			followingObj = node;
			node->precedingObj = this;
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

	void draw() {
		ofSetColor(255);
		ofSetLineWidth(10);
		if (dobj && followingObj) {
			ofDrawLine(dobj->getX(), dobj->getY(), followingObj->dobj->getX(), followingObj->dobj->getY());
		}
	}



	virtual void updateAngleValue(float angle) {};


	void setDirectObject(DirectObject* dobj) {
		this->dobj = dobj;
	}

	DirectObject* getDirectObject() {
		return dobj;
	}

	float getDistanceTo(TableObject* obj) {
		return this->dobj->getDistance(obj->dobj);
	}

	


protected:

private:
	int		id;
	float	rawAngleLastValue = 0.0f;
	int		turnsMultiplier = 1;
	const float derivativeThreshold = 1.0f;
	DirectObject* dobj;
	TableObject* followingObj;
	TableObject* precedingObj = nullptr;
};

class Generator : public TableObject {

public:

	Generator(int id = -1) : TableObject(id) {
		patch();
	}
	Generator(const Generator & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void patch() {
		addModuleInput("signal", input);
		addModuleInput("pitch", pitch);
		addModuleInput("trig", trig);
		addModuleOutput("signal", amp);

		//patching
		osc.out_saw() >> amp;
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


private:
	pdsp::PatchNode     input;
	pdsp::PatchNode     pitch;
	pdsp::PatchNode     trig;
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
		addModuleInput("input", input);
		addModuleOutput("output", amp);

		input >> filter >> amp;
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
	pdsp::PatchNode     input;
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
		addModuleInput("input", input);

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
	pdsp::Engine        engine;
	pdsp::PatchNode     input;

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
		pdsp::Engine            engine;
};

#endif
