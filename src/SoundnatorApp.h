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
#include "InputGestureTap.hpp"
#include "Alarm.hpp"

class Generator;
class Effect;
class Controller;
class Global;
class Output;

static pdsp::Engine engine;

typedef enum { AUDIO, CONTROL } connectionType_t;

class TableObject : public pdsp::Patchable, public Graphic {
public:
	TableObject(int id = -1, connectionType_t connection = AUDIO) : id(id), dobj(nullptr), connection(connection), followingObj(nullptr), precedingAudioObj(nullptr), precedingControlObj(nullptr) {
		registerMyEvent(InputGestureDirectFingers::I().newCursor, &TableObject::addCursor, this);

		registerMyEvent(InputGestureTap::I().Tap, &TableObject::Tap, this);

		registerEvent(InputGestureDirectObjects::I().updateObject, &TableObject::updateObject, this);

		addModuleInput("signal", input);
		addModuleInput("pitch", pitch_in);
		addModuleInput("trig", trig_in);
		addModuleOutput("signal", output);
		addModuleOutput("pitch", pitch_out);
		addModuleOutput("trig", trig_out);

		scope.set(bufferLen);
	}

	bool Collide(ofPoint const & point)
	{
		if (getDirectObject()) {
			float dist = point.distance(ofVec3f(getDirectObject()->getX(), getDirectObject()->getY()));
			return (dist <= 0.2);
		}
		else
			return false;
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

	virtual void addCursor(InputGestureDirectFingers::newCursorArgs & a) {
	}

	virtual void Tap(InputGestureTap::TapArgs & a) {

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


	TableObject** getPrecedingObj(TableObject* obj) {
		return getPrecedingObj(obj, this->connection);
	}

	TableObject** getPrecedingObj(TableObject* obj, connectionType_t connection) {
		switch (connection)
		{
		case AUDIO:
			return &obj->precedingAudioObj;
			break;
		case CONTROL:
			return &obj->precedingControlObj;
			break;
		default:
			break;
		}
		return nullptr;
	}

	void makeConnectionTo(TableObject* obj) {
		switch (connection)
		{
		case AUDIO:
			*this >> *obj;
			break;
		case CONTROL:
			this->out("pitch") >> obj->in("pitch");
			this->out("trig") >> obj->in("trig");
			break;
		default:
			break;
		}
	}




	bool isConnectedTo(TableObject* obj) {
		return obj == followingObj;
	}

	virtual bool canConnectTo(TableObject* obj) {
		if (isObject<Output>(this)) {
			return false;
		}
		else {

			TableObject** objPrecedingObj = getPrecedingObj(obj);

			if (isObject<Output>(obj) && objectIsConnectableToOutput()) {
				return true;
			}
			else if ((*objPrecedingObj == nullptr) && objectIsConnectableTo(obj)) {
				return true;
			}
			else {
				return false;
			}
		}
	}

	virtual bool objectIsConnectableTo(TableObject* obj) = 0;

	virtual bool objectIsConnectableToOutput() = 0;





	void connectTo(TableObject* obj) {

		if (followingObj) {
			*getPrecedingObj(followingObj) = nullptr;
		}
		this->disconnectOut();

		makeConnectionTo(obj);

		followingObj = obj;

		*getPrecedingObj(obj) = this;
	}





	virtual bool haveConnection() {
		return (this->followingObj != nullptr);
	}



	void makeDisconnectionTo(TableObject* obj, connectionType_t connection) {
		TableObject** precedingObj = getPrecedingObj(this, connection);
		(*precedingObj)->disconnectOut();
		followingObj->disconnectIn();

		**precedingObj >> *followingObj;
		(*precedingObj)->followingObj = followingObj;
		*getPrecedingObj(followingObj, connection) = *precedingObj;

		precedingObj = nullptr;
		followingObj = nullptr;
	}

	void remove() {

		if (precedingAudioObj && precedingControlObj && followingObj) {
			precedingAudioObj->disconnectOut();
			precedingControlObj->disconnectOut();

			precedingAudioObj->followingObj = nullptr;
			precedingControlObj->followingObj = nullptr;

			precedingAudioObj = nullptr;
			precedingControlObj = nullptr;
		}
		else if (precedingAudioObj && followingObj) {
			precedingAudioObj->disconnectOut();
			followingObj->disconnectIn();
			*precedingAudioObj >> *followingObj;
			precedingAudioObj->followingObj = followingObj;
			followingObj->precedingAudioObj = precedingAudioObj;
			precedingAudioObj = nullptr;
		}
		else if (precedingControlObj && followingObj) {
			precedingControlObj->disconnectOut();
			followingObj->disconnectIn();
			precedingControlObj->followingObj = nullptr;
			precedingControlObj = nullptr;
		}
		if (followingObj) {
			*getPrecedingObj(followingObj) = nullptr;
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

	virtual void objectDraw() {}



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
	pdsp::PatchNode     pitch_in;
	pdsp::PatchNode     pitch_out;
	pdsp::PatchNode     trig_in;
	pdsp::PatchNode     trig_out;

private:
	int		id;
	float	rawAngleLastValue = 0.0f;
	int		turnsMultiplier = 1;
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

	Generator(int id = -1, connectionType_t connection = AUDIO) : TableObject(id, connection) {
		patch();
	}
	Generator(const Generator & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void patch() {

		//patching
		osc.out_saw() >> amp >> output;
		env >> amp.in_mod();
		trig_in >> env;
		pitch_ctrl >> osc.in_pitch();
		pitch_ctrl.enableSmoothing(50.0f);
		amp.set(1.0f);
		trig_in.set(1.0f);
		this->setToScope(amp);
	}

	void updateAngleValue(float angle) {
		float pitch = ofMap(angle, 0, 5.0f * M_2PI, 36.0f, 96.0f);
		pitch_ctrl.set(pitch);
	}

	bool objectIsConnectableTo(TableObject* obj) {
		return isObject<Effect>(obj);
	}

	bool objectIsConnectableToOutput() {
		return true;
	}

	void Tap(InputGestureTap::TapArgs & a) {
		cout << "TAP" << endl;
		switch (choose % 2) {
		case 0:
			osc.disconnectOut();
			osc.out_saw() >> amp;
			break;
		case 1:
			osc.disconnectOut();
			osc.out_sine() >> amp;
			break;
		}
		choose++;
	}

	void objectDraw() {

	}

private:
	int choose = 0;
	pdsp::ValueControl  pitch_ctrl;
	pdsp::Amp           amp;
	pdsp::VAOscillator  osc;
	pdsp::ADSR			env;

};

class Effect : public TableObject {

public:

	Effect(int id = -1, connectionType_t connection = AUDIO) : TableObject(id, connection) {
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

	bool objectIsConnectableTo(TableObject* obj) {
		return isObject<Effect>(obj);
	}

	bool objectIsConnectableToOutput() {
		return true;
	}


private:
	pdsp::Amp           amp;
	pdsp::ValueControl	cutoff_ctrl;
	pdsp::VAFilter      filter; // 24dB multimode filter

};


class Controller : public TableObject {

public:

	Controller(int id = -1, connectionType_t connection = CONTROL) : TableObject(id, CONTROL) {
		patch();
	}
	Controller(const Controller  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile


	void patch() {
		pitch_ctrl >> osc;
		osc.out_pulse() >> amp >> trig_out;
		this->setToScope(amp);
		amp.set(1.0f);
	}

	void addCursor(InputGestureDirectFingers::newCursorArgs & a) {
	}

	void updateAngleValue(float angle) {
		float pitch = ofMap(angle, 0, 5.0f * M_2PI, 0.0f, 36.0f);
		pitch_ctrl.set(pitch);
	}

	bool objectIsConnectableTo(TableObject* obj) {
		return isObject<Generator>(obj) || isObject<Effect>(obj);
	}

	bool objectIsConnectableToOutput() {
		return false;
	}


private:
	pdsp::Amp           amp;
	pdsp::VAOscillator  osc;
	pdsp::ValueControl	pitch_ctrl;
	pdsp::VAFilter      filter; // 24dB multimode filter

};



////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	bool haveConnection() {
		return true;
	}

	bool objectIsConnectableTo(TableObject* obj) {
		return false;
	}

	bool objectIsConnectableToOutput() {
		return false;
	}

private:
};





/////////////////////////////////////////////////////////////////////////////////////////////////////////

class SoundnatorApp : public ofBaseApp {

public:

	TableApp tableapp;
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
};

#endif
