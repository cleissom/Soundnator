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
class Effects;
class Controller;
class Global;
class Output;

class TableObject : public pdsp::Patchable, public Graphic {
public:
	TableObject(int id = -1) : id(id), dobj(nullptr), followingObj(nullptr) {
		registerEvent(InputGestureDirectFingers::I().enterCursor, &TableObject::addCursor, this);
		registerEvent(InputGestureDirectObjects::I().updateObject, &TableObject::updateObject, this);
	}

	void updateTurnMultiplier(float angle) {
		float derivative = angle - rawAngleLastValue;
		if (derivative > derivativeThreshold) turnsMultiplier--;
		else if (derivative < (-derivativeThreshold)) turnsMultiplier++;
		rawAngleLastValue = angle;
	}

	void addCursor(InputGestureDirectFingers::newCursorArgs & a) {
		cout << "cursor input" << endl;
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

	void connectTo(TableObject* node) {
		connectToIfType<Effects>(node);
		connectToIfType<Output>(node);

	}

	void remove() {
		if (precedingObj) {
			precedingObj->disconnectOut();
			followingObj->disconnectIn();
			*precedingObj >> *followingObj;
			precedingObj->followingObj = followingObj;
		}
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

	DirectObject* dobj;
protected:

private:
	int		id;
	float	rawAngleLastValue = 0.0f;
	int		turnsMultiplier = 1;
	const float derivativeThreshold = 1.0f;
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


private:
	pdsp::PatchNode     input;
	pdsp::PatchNode     pitch;
	pdsp::PatchNode     trig;
	pdsp::ValueControl  pitch_ctrl;
	pdsp::Amp           amp;
	pdsp::VAOscillator  osc;
};

class Effects : public TableObject {

public:

	Effects(int id = -1) : TableObject(id) {
		patch();
	}
	Effects(const Effects  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile


	void patch() {
		addModuleInput("input", input);
		addModuleOutput("output", amp);

		input >> filter >> amp;
		cutoff_ctrl >> filter.in_cutoff();
		amp.set(1.0f);
	}

	void addCursor(InputGestureDirectFingers::newCursorArgs & a) {
		cout << "cursor input" << endl;
	}

	void updateAngleValue(float angle) {
		float cutoff = ofMap(angle, 0, 2.0f * M_2PI, 48.0f, 96.0f);
		cutoff_ctrl.set(cutoff);
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
		dobj = new DirectObject();
		dobj->s_id = -1;
		dobj->f_id = id;
		dobj->setX(1.0f);
		dobj->setY(0.5f);
		dobj->angle = 0;
		dobj->xspeed = 0;
		dobj->yspeed = 0;
		dobj->rspeed = 0;
		dobj->maccel = 0;
		dobj->raccel = 0;

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
		ofDrawCircle(dobj->getX(),dobj->getY(), 0.03f);
		ofDrawLine(0.0f, 0.5f, 1.0f, 0.5f);
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
