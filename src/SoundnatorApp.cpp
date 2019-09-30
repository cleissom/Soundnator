#include "SoundnatorApp.h"

#include "CursorFeedback.hpp"
#include "FigureFeedback.hpp"
#include "TapFeedback.hpp"
#include "LongPushFeedback.hpp"



#include "InputGestureDirectFingers.hpp"
#include "FigureGraphic.hpp"
#include "Alarm.hpp"

#include "SoundDispatcher.h"

class Test2 : public Graphic
{
	Figures::Polygon polygon;
	FigureGraphic * fg;
	float time_circle;
	pdsp::Engine            engine;
	pdsp::VAOscillator      osc;
public:
	Test2()
	{
		polygon.AddVertex(ofPoint(-0.05f, -0.05f));
		polygon.AddVertex(ofPoint(-0.05f, 0.05f));
		polygon.AddVertex(ofPoint(0.05f, 0.05f));
		polygon.AddVertex(ofPoint(0.05f, 0.025f));
		polygon.AddVertex(ofPoint(-0.025f, 0.025f));
		polygon.AddVertex(ofPoint(-0.025f, -0.025f));
		polygon.AddVertex(ofPoint(0.025f, -0.025f));
		polygon.AddVertex(ofPoint(0.025f, 0.0f));
		polygon.AddVertex(ofPoint(0.05f, 0.0f));
		polygon.AddVertex(ofPoint(0.05f, -0.05f));

		//polygon.SetTexture("temp.png");

		fg = new FigureGraphic(&polygon);
		fg->transformation.setTranslation(ofRandom(0, 1), ofRandom(0, 1), 0);
		fg->transformation.glRotate(ofRandom(0, 360), 0, 0, 1);
		//registerEvent(InputGestureBasicFingers::Instance().enterTuioCursor, &Test2::genericCallback);
		fg->registerMyEvent(InputGestureDirectFingers::Instance().enterCursor, &Test2::enter, this);
		//fg->registerMyEvent(InputGestureDirectFingers::Instance().exitCursor, &Test2::genericCallback2,this);

		time_circle = ofRandom(0.1, 2);

		fg->color.r = ofRandom(0, 255);
		fg->color.g = ofRandom(0, 255);
		fg->color.b = ofRandom(0, 255);
		fg->color.a = 100;
		fg->isHidden(true);

		osc.out_sine() * dB(-12.0f) >> engine.audio_out(0); // connect to left output channel
		osc.out_sine() * dB(-12.0f) >> engine.audio_out(1); // connect to right right channel

		engine.setDeviceID(1); // REMEMBER TO SET THIS AT THE RIGHT INDEX!!!!

	//engine.setApi( ofSoundDevice::Api::PULSE ); // use this if you need to change the API

	// start your audio engine !
		engine.setup(44100, 512, 3);
	}

	void enter(InputGestureDirectFingers::enterCursorArgs & e)
	{
		fg->isHidden(false);
		fg->hasAlpha(true);
		//fg->canCollide(false);
		fg->setFill(false);
		Alarm::Cancel(this);
		Alarm::Setup(ofGetElapsedTimef() + 4, this, &Test2::alive);
	}
	void alive(float & t)
	{
		fg->hasAlpha(false);
		fg->canCollide(true);
		fg->setFill(true);
		fg->color.r = ofRandom(0, 255);
		fg->color.g = ofRandom(0, 255);
		fg->color.b = ofRandom(0, 255);
		Alarm::Setup(ofGetElapsedTimef() + 4, this, &Test2::die);
	}

	void die(float & t)
	{
		fg->isHidden(true);
		fg->canCollide(true);
	}

	void update()
	{
		fg->transformation.glRotate(1, 0, 0, 1);
	}
};

class Test : public Graphic
{
	Figures::Polygon polygon;
	FigureGraphic * fg;
	float time_circle;
public:
	Test()
	{
		polygon.AddVertex(ofPoint(-0.05f, -0.05f));
		polygon.AddVertex(ofPoint(-0.05f, 0.05f));
		polygon.AddVertex(ofPoint(0.05f, 0.05f));
		polygon.AddVertex(ofPoint(0.05f, 0.025f));
		polygon.AddVertex(ofPoint(-0.025f, 0.025f));
		polygon.AddVertex(ofPoint(-0.025f, -0.025f));
		polygon.AddVertex(ofPoint(0.025f, -0.025f));
		polygon.AddVertex(ofPoint(0.025f, 0.0f));
		polygon.AddVertex(ofPoint(0.05f, 0.0f));
		polygon.AddVertex(ofPoint(0.05f, -0.05f));

		//polygon.SetTexture("temp.png");

		fg = new FigureGraphic(&polygon);
		fg->transformation.setTranslation(ofRandom(0, 1), ofRandom(0, 1), 0);
		fg->transformation.glRotate(ofRandom(0, 360), 0, 0, 1);
		//registerEvent(InputGestureBasicFingers::Instance().enterTuioCursor, &Test2::genericCallback);
		fg->registerMyEvent(InputGestureDirectFingers::Instance().enterCursor, &Test::enter, this);
		//fg->registerMyEvent(InputGestureDirectFingers::Instance().exitCursor, &Test2::genericCallback2,this);

		time_circle = ofRandom(0.1, 2);

		fg->color.r = ofRandom(0, 255);
		fg->color.g = ofRandom(0, 255);
		fg->color.b = ofRandom(0, 255);
		fg->color.a = 100;
		fg->isHidden(true);
	}

	void enter(InputGestureDirectFingers::enterCursorArgs & e)
	{
		fg->isHidden(false);
		fg->hasAlpha(true);
		//fg->canCollide(false);
		fg->setFill(false);
		Alarm::Cancel(this);
		Alarm::Setup(ofGetElapsedTimef() + 4, this, &Test::alive);
	}
	void alive(float & t)
	{
		fg->hasAlpha(false);
		fg->canCollide(true);
		fg->setFill(true);
		fg->color.r = ofRandom(0, 255);
		fg->color.g = ofRandom(0, 255);
		fg->color.b = ofRandom(0, 255);
		Alarm::Setup(ofGetElapsedTimef() + 4, this, &Test::die);
	}

	void die(float & t)
	{
		fg->isHidden(true);
		fg->canCollide(true);
	}

	void update()
	{
		fg->transformation.glRotate(1, 0, 0, 1);
	}
};

//--------------------------------------------------------------
void SoundnatorApp::setup() {

	tableapp.setup();

	for (int i = 0; i < 500; ++i)
		new Test();
	new CursorFeedback();
	new FigureFeedback();
	new TapFeedback();
	new LongPushFeedback();
	//new Test2();
	new SoundDispatcher();

}

//--------------------------------------------------------------
void SoundnatorApp::update() {
}

//--------------------------------------------------------------
void SoundnatorApp::draw() {
	tableapp.draw();
}

//--------------------------------------------------------------
void SoundnatorApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void SoundnatorApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void SoundnatorApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void SoundnatorApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void SoundnatorApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void SoundnatorApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void SoundnatorApp::windowResized(int w, int h) {

}

