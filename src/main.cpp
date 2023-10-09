#include "SoundnatorApp.h"
#include "ofMain.h"
//#include "ofAppGlutWindow.h"
#include "ofxGlobalConfig.hpp"

int main() {
    // ofAppGlutWindow window;
    int width = ofxGlobalConfig::getRef("PROGRAM:WIDTH", 1024);
    int height = ofxGlobalConfig::getRef("PROGRAM:HEIGHT", 768);
    if (ofxGlobalConfig::getRef("PROGRAM:FULLSCREEN", 0))
        ofSetupOpenGL(width, height, OF_FULLSCREEN);
    else
        ofSetupOpenGL(width, height, OF_WINDOW);

    // ofGLWindowSettings settings;
    // auto window = ofCreateWindow(settings);
    //ofSetupOpenGL(width,height, OF_GAME_MODE);
    //ofSetupOpenGL(&window, width ,height, OF_WINDOW);

    ofRunApp(new SoundnatorApp());
}
