#ifndef _SOUNDNATOR_APP
#define _SOUNDNATOR_APP

#include "Alarm.hpp"
#include "CollisionHelper.h"
#include "Figure.h"
#include "FigureGraphic.hpp"
#include "InputGestureDirectFingers.hpp"
#include "InputGestureDirectObjects.hpp"
#include "InputGestureTap.hpp"
#include "Polygon.h"
#include "TableApp.hpp"
#include "TableObject.h"
#include "ofMain.h"
#include "ofxPDSP.h"

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
