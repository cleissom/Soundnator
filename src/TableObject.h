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

	virtual void addCursor(InputGestureDirectFingers::newCursorArgs & a);
	virtual void Tap(InputGestureTap::TapArgs & a);
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

	Generator(int id = -1, connectionType_t connection = AUDIO);
	Generator(const Generator & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void enter(InputGestureDirectFingers::enterCursorArgs& a);
	void update();
	void patch();
	void updateAngleValue(float angle);
	bool objectIsConnectableTo(TableObject* obj);
	bool objectIsConnectableToOutput();
	void Tap(InputGestureTap::TapArgs & a);
	void objectDraw();

private:
	int choose = 0;
	Figures::Polygon polygon;
	FigureGraphic* fg;
	pdsp::ValueControl  pitch_ctrl;
	pdsp::Amp           amplifier;
	pdsp::VAOscillator  osc;
	pdsp::ADSR			env;

};


class Effect : public TableObject {

public:

	Effect(int id = -1, connectionType_t connection = AUDIO);
	Effect(const Effect  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile
	void patch();

	void addCursor(InputGestureDirectFingers::newCursorArgs & a);
	void updateAngleValue(float angle);
	bool objectIsConnectableTo(TableObject* obj);
	bool objectIsConnectableToOutput();


private:
	pdsp::Amp           amp;
	pdsp::ValueControl	cutoff_ctrl;
	pdsp::VAFilter      filter; // 24dB multimode filter

};


class Controller : public TableObject {

public:

	Controller(int id = -1, connectionType_t connection = CONTROL);
	Controller(const Controller  & other) { patch(); } // you need this to use std::vector with your class, otherwise will not compile

	void patch();
	void addCursor(InputGestureDirectFingers::newCursorArgs & a);
	void updateAngleValue(float angle);
	bool objectIsConnectableTo(TableObject* obj);
	bool objectIsConnectableToOutput();


private:
	pdsp::Amp           amp;
	pdsp::VAOscillator  osc;
	pdsp::ValueControl	pitch_ctrl;
	pdsp::VAFilter      filter; // 24dB multimode filter

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

