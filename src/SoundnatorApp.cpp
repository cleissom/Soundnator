#include "SoundnatorApp.h"

#include "CursorFeedback.hpp"
#include "FigureFeedback.hpp"
#include "TapFeedback.hpp"
#include "LongPushFeedback.hpp"



#include "InputGestureDirectFingers.hpp"
#include "FigureGraphic.hpp"
#include "Alarm.hpp"

const int OUTPUT = 216;

class SoundDispatcher : public EventClient {
	
	std::map<int, TableObject*> TableObjects;
	std::map<int, DirectObject *> ObjectsOnTable;

public: 
	SoundDispatcher() {
		TableObjects.insert(std::make_pair(0, new Generator(0)));
		TableObjects.insert(std::make_pair(1, new Generator(1)));
		TableObjects.insert(std::make_pair(2, new Effects(2)));
		TableObjects.insert(std::make_pair(3, new Effects(3)));
		TableObjects.insert(std::make_pair(OUTPUT, new Output(OUTPUT)));

		registerEvent(InputGestureDirectFingers::I().enterCursor, &SoundDispatcher::addCursor, this);

		registerEvent(InputGestureDirectObjects::I().newObject, &SoundDispatcher::newObject, this);
		registerEvent(InputGestureDirectObjects::I().enterObject, &SoundDispatcher::enterObject, this);
		registerEvent(InputGestureDirectObjects::I().updateObject, &SoundDispatcher::updateObject, this);
		registerEvent(InputGestureDirectObjects::I().removeObject, &SoundDispatcher::exitObject, this);

		

	}
	void addCursor(InputGestureDirectFingers::newCursorArgs & a) {
	}

	void updateCursor(InputGestureDirectFingers::updateCursorArgs & a) {
	}


	void newObject(InputGestureDirectObjects::newObjectArgs& a) {
		cout << "New object" << endl;
		int id = a.object->f_id;
		ObjectsOnTable[id] = a.object;
		TableObjects[id]->dobj = a.object;
	}

	void enterObject(InputGestureDirectObjects::enterObjectArgs& a) {
		cout << "Enter object" << endl;
		int id = a.object->f_id;

		int somethingAhead = hasSomethingAhead(ObjectsOnTable[id]);
		int somethingBehind = hasSomethingBehind(ObjectsOnTable[id]);

		if (somethingAhead != -1) {
			TableObjects[id]->connectTo(TableObjects[somethingAhead]);
			cout << "Something ahead" << endl;
		}
		else {
			TableObjects[id]->connectTo(TableObjects[OUTPUT]);
		}

		if (somethingBehind != -1) {
			TableObjects[somethingBehind]->connectTo(TableObjects[id]);
			cout << "Something behind" << endl;
		}
		

	}

	int hasSomethingAhead(DirectObject* obj) {
		for (std::map<int, DirectObject *>::iterator it = ObjectsOnTable.begin(); it != ObjectsOnTable.end(); ++it) {
			if (obj != it->second)
				if (it->second->getX() > obj->getX() && ((it->second->getY() >= 0.5f && obj->getY() >= 0.5f) || (it->second->getY() < 0.5f && obj->getY() < 0.5f))) return it->first;
		}
		return -1;
	}
	int hasSomethingBehind(DirectObject* obj) {
		for (std::map<int, DirectObject *>::iterator it = ObjectsOnTable.begin(); it != ObjectsOnTable.end(); ++it) {
			if (obj != it->second)
				if (it->second->getX() < obj->getX() && ((it->second->getY() >= 0.5f && obj->getY() >= 0.5f) || (it->second->getY() < 0.5f && obj->getY() < 0.5f))) return it->first;
		}
		return -1;
	}

	void updateObject(InputGestureDirectObjects::updateObjectArgs& a) {
	}

	void exitObject(InputGestureDirectObjects::exitObjectArgs& a) {
		cout << "Exit object" << endl;
		int id = a.object->f_id;
		ObjectsOnTable.erase(id);
		TableObjects[id]->remove();
		TableObjects[id]->dobj = nullptr;
	}
	
};

class Test2 : public CanDirectObjects<Graphic> {
	// pdsp modules
	pdsp::Engine            engine;
	pdsp::VAOscillator      osc;
	pdsp::LFO               lfo;
	pdsp::ValueControl      pitch_ctrl;
	pdsp::ValueControl      amp_ctrl;
	pdsp::ValueControl      filter_ctrl;
	pdsp::Amp               amp;
	pdsp::ADSR              env;
	pdsp::TriggerControl    gate_ctrl;

	std::vector<pdsp::ValueControl>    pitches;
	std::vector<pdsp::LFO>          drift_lfo;
	std::vector <pdsp::ValueControl>               amps;
	std::vector<pdsp::ValueControl>      filters;

	int choice = 0;

public:
	Test2() {
		/*
		pitch_ctrl >> synth.in("pitch"); // patching with in("tag")
		amp_ctrl >> synth.in_amp(); // patching with custom in_tag()
		synth * dB(-12.0f) >> engine.audio_out(0);
		synth * dB(-12.0f) >> engine.audio_out(1);

		filter_ctrl >> synth.in("filter");

		pitch_ctrl.set(60.0f);
		pitch_ctrl.enableSmoothing(50.0f); // 50ms smoothing
		amp_ctrl.set(0.0f);
		amp_ctrl.enableSmoothing(50.0f); // 50ms smoothing
		*/
	}

	void newObject(DirectObject * object) {
		/*
		int i = object->f_id;
		float pitch = ofMap(object->getX(), 0, 1.0f, 36.0f, 84.0f);
		pitches[i].set(pitch);
		float amp = ofMap(object->getY(), 0, 1.0f, 1.5f, 0.0f);
		amps[i].set(amp);
		float freq = ofMap(object->angle, 0, M_2PI, 50.0f, 150.0f);
		filters[i].set(freq);
		*/
	}

	void updateObject(DirectObject * object) {


	}

	void removeObject(DirectObject * object) {
		osc.disconnectAll();
	}
};



class Test: public Graphic
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
        fg->transformation.setTranslation(ofRandom(0,1),ofRandom(0,1),0);
        fg->transformation.glRotate(ofRandom(0,360),0,0,1);
        //registerEvent(InputGestureBasicFingers::Instance().enterTuioCursor, &Test2::genericCallback);
        fg->registerMyEvent(InputGestureDirectFingers::Instance().enterCursor, &Test::enter,this);
        //fg->registerMyEvent(InputGestureDirectFingers::Instance().exitCursor, &Test2::genericCallback2,this);
        
        time_circle = ofRandom(0.1,2);
        
        fg->color.r = ofRandom(0,255);
        fg->color.g = ofRandom(0,255);
        fg->color.b = ofRandom(0,255);
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
        Alarm::Setup(ofGetElapsedTimef()+4,this,&Test::alive);
    }
    void alive(float & t)
    {
        fg->hasAlpha(false);
        fg->canCollide(true);
        fg->setFill(true);
        fg->color.r = ofRandom(0,255);
        fg->color.g = ofRandom(0,255);
        fg->color.b = ofRandom(0,255);
        Alarm::Setup(ofGetElapsedTimef()+4,this,&Test::die);
    }
    
    void die(float & t)
    {
        fg->isHidden(true);
        fg->canCollide(true);
    }
    
    void update()
    {
        fg->transformation.glRotate(1,0,0,1);
    }
};

//--------------------------------------------------------------
void SoundnatorApp::setup(){

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
void SoundnatorApp::update(){
}

//--------------------------------------------------------------
void SoundnatorApp::draw(){
    tableapp.draw();
}

//--------------------------------------------------------------
void SoundnatorApp::keyPressed(int key){

}

//--------------------------------------------------------------
void SoundnatorApp::keyReleased(int key){

}

//--------------------------------------------------------------
void SoundnatorApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void SoundnatorApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void SoundnatorApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void SoundnatorApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void SoundnatorApp::windowResized(int w, int h){

}

