#ifndef _FONT
#define _FONT

#include "Singleton.hpp"
#include "ofMain.h"

namespace utils {
class Font {
   public:
    Font(int fontsize = 10) { font.load("verdana.ttf", fontsize, true); }

    void drawString(string str, bool center = true, float x = 0, float y = 0, bool bottom = false) {
        ofPushMatrix();

        int shortside = min(ofGetWidth(), ofGetHeight());
        float xScaled = -x * shortside;
        float yScaled = -y * shortside;
        glRotatef(180.0, 0, 0, 1);
        glScalef(1.0f / shortside, 1.0f / shortside, 1);
        auto xoffset = center ? -font.getStringBoundingBox(str, 0, 0).width / 2.0 : 0.0f;
        auto yoffset = bottom ? font.getStringBoundingBox(str, 0, 0).height : 0.0f;

        glScalef(-1.0f, 1.0, 1);
        font.drawString(str, xScaled + xoffset, yScaled + yoffset);

        ofPopMatrix();
    }

   private:
    ofTrueTypeFont font;
};
}  // namespace utils

#endif  // !_FONT
