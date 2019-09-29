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
	std::set<TableObject* > ObjectsOnTable;

public: 
	SoundDispatcher() {
		TableObjects.insert(std::make_pair(0, new Generator(0)));
		TableObjects.insert(std::make_pair(1, new Generator(1)));
		TableObjects.insert(std::make_pair(2, new Effect(2)));
		TableObjects.insert(std::make_pair(3, new Effect(3)));
		TableObjects.insert(std::make_pair(4, new Controller(4)));
		TableObjects.insert(std::make_pair(OUTPUT, new Output(OUTPUT)));

		registerEvent(InputGestureDirectFingers::I().enterCursor, &SoundDispatcher::addCursor, this);

		registerEvent(InputGestureDirectObjects::I().newObject, &SoundDispatcher::newObject, this);
		registerEvent(InputGestureDirectObjects::I().enterObject, &SoundDispatcher::enterObject, this);
		registerEvent(InputGestureDirectObjects::I().updateObject, &SoundDispatcher::updateObject, this);
		registerEvent(InputGestureDirectObjects::I().removeObject, &SoundDispatcher::exitObject, this);

		addOutputToTable();

	}
	void addCursor(InputGestureDirectFingers::newCursorArgs & a) {
	}

	void updateCursor(InputGestureDirectFingers::updateCursorArgs & a) {
	}

	void addOutputToTable() {
		DirectObject* dobj= new DirectObject();
		dobj->s_id = -1;
		dobj->f_id = OUTPUT;
		dobj->setX(1.0f);
		dobj->setY(0.5f);
		dobj->angle = 0;
		dobj->xspeed = 0;
		dobj->yspeed = 0;
		dobj->rspeed = 0;
		dobj->maccel = 0;
		dobj->raccel = 0;

		int id = dobj->f_id;
		TableObjects[id]->setDirectObject(dobj);
		ObjectsOnTable.insert(TableObjects[id]);
	}

	void addObjectToTable(TableObject* obj) {
		if (ObjectsOnTable.find(obj) == ObjectsOnTable.end()) {
			ObjectsOnTable.insert(obj);
		}
	}

	void addObjectToTable(DirectObject* dobj) {
		int id = dobj->f_id;
		if (ObjectsOnTable.find(TableObjects[id]) == ObjectsOnTable.end()) {
			TableObjects[id]->setDirectObject(dobj);
			ObjectsOnTable.insert(TableObjects[id]);
		}
		
	}

	void removeObjectFromTable(DirectObject* dobj) {
		int id = dobj->f_id;
		if (ObjectsOnTable.find(TableObjects[id]) != ObjectsOnTable.end()) {
			TableObjects[id]->remove();
			ObjectsOnTable.erase(TableObjects[id]);
		}
		
	}



	std::pair<TableObject*, TableObject*> findShorterDistancesFrom(TableObject* obj) {
		float first =  10000;
		float second = 10001;
		std::pair<TableObject*, TableObject*> shorter(nullptr,nullptr);

		cout << "Finding Shorter Distance from " << obj->getId();

		for (TableObject* object : ObjectsOnTable) {
			cout << object->getId() << endl;
			if (object != obj) {
				float dist = obj->getDistanceTo(object);
				if (dist < first) {
					second = first;
					std::swap(shorter.first, shorter.second);
					first = dist;
					shorter.first = object;
				}
				else if (dist < second) {
					second = dist;
					shorter.second = object;
				}
			}
		}

		if (shorter.second) {
			cout << ". first: " << shorter.first->getId() << ". second: " << shorter.second->getId() << endl;
		}
		else if (shorter.first) {
			cout << ". first: " << shorter.first->getId() << endl;
		}

		return shorter;
	}

	void processConnections(TableObject* object) {
		cout << "processConnection " << object->getId() << endl;

		std::pair<TableObject*, TableObject*> shorter = findShorterDistancesFrom(object);

		if (shorter.first) {
			cout << "have first. ";
			if (shorter.second) {
				cout << "have second. ";
				shorter.second->isConnectedTo(shorter.first) ? (cout << "second is connected. ") : (cout << "second is not connected. ");

				object->canConnectTo(shorter.first) ? (cout << "can connect. ") : (cout << "can not connect. ");

				if (object->canConnectTo(shorter.first)) {

					if (shorter.second->isConnectedTo(shorter.first)) {
						auto distSecondToFirst = shorter.second->getDistanceTo(shorter.first);
						auto distObjToFirst = object->getDistanceTo(shorter.first);
						auto distObjToSecond = object->getDistanceTo(shorter.second);

						if ((distSecondToFirst > distObjToFirst) && (distObjToSecond < distSecondToFirst)) {
							cout << "distance. ";
							if (shorter.second->canConnectTo(object)) {
								shorter.second->connectTo(object);
							}
						}
					}

					if (shorter.first->haveConnection()) {
						object->connectTo(shorter.first);
					}
				}
			}
			else {
				cout << "not have second. ";
				if (object->canConnectTo(shorter.first)) {
					object->connectTo(shorter.first);
				}
			}
		}

		cout << endl;
		
	}
	


	void newObject(InputGestureDirectObjects::newObjectArgs& a) {
		cout << "New object" << endl;
		cout << " \n" << endl;
		cout << " \n" << endl;
		int id = a.object->f_id;
		addObjectToTable(a.object);
		processConnections(TableObjects[a.object->f_id]);
	}

	void enterObject(InputGestureDirectObjects::enterObjectArgs& a) {
		cout << "Enter object" << endl;
		

	}

	void updateObject(InputGestureDirectObjects::updateObjectArgs& a) {
		processConnections(TableObjects[a.object->f_id]);
	}

	void exitObject(InputGestureDirectObjects::exitObjectArgs& a) {
		cout << "Exit object" << endl;
		removeObjectFromTable(a.object);
	}
	
};

class Test2 : public CanDirectObjects<Graphic> {
	// pdsp modules
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

