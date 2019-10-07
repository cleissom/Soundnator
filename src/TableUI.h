#ifndef _TABLEUI
#define _TABLEUI

#include "Figure.h"
#include "CollisionHelper.h"
#include "Polygon.h"
#include "FigureGraphic.hpp"
#include "InputGestureDirectFingers.hpp"
#include "InputGestureDirectObjects.hpp"
#include "InputGestureTap.hpp"


class TableUIBase : public Graphic {

public:
	TableUIBase(float angle = 0.0f, float distanceOffset = 0.05f);
	void registerFingerEvents(FigureGraphic* fg);
	virtual void updatePosition(float x, float y);
	virtual void updateAngle(float angle);
	virtual void updateDistanceOffset(float distance);
	virtual void updateTransformationMatrix() {};

	virtual void fingersEnter(InputGestureDirectFingers::enterCursorArgs& a) {};
	virtual void fingersUpdate(InputGestureDirectFingers::updateCursorArgs& a) {};
	virtual void fingersTap(InputGestureTap::TapArgs& a) {};

	virtual void isHidden(bool) = 0;

	void setDistanceOffset(float dist) { this->distanceOffset = dist; };
	void setAngle(float ang) { this->angle = ang; };
	float getX() { return this->x_center; };
	float getY() { return this->y_center; };
	float getDistanceOffset() { return this->distanceOffset; };
	float getAngle() { return this->angle; };

protected:
	float x_center, y_center;
	float distanceOffset, angle;
};





class TableButton : public TableUIBase {
	struct commomTableButtonArgs : public EventArgs
	{
		int id;
	};

public:
	typedef commomTableButtonArgs TapButtonArgs;

	ofEvent<TapButtonArgs> TapButton;

	TableButton(float angle = 0.0f, float distanceOffset = 0.05f, float size = 2.0f);
	void updateTransformationMatrix();
	void isHidden(bool);

	void fingersEnter(InputGestureDirectFingers::enterCursorArgs & a);
	void fingersTap(InputGestureTap::TapArgs & a);

private:
	FigureGraphic* base;
	float buttonSize;
};





class TableSlider : public TableUIBase {
	struct commomTableSliderArgs : public EventArgs
	{
		float percentage;
	};

public:
	typedef commomTableSliderArgs updateSliderArgs;

	ofEvent<updateSliderArgs> updateSlider;

	TableSlider(float angle = 0.0f, float distanceOffset = 0.075f, float sliderSize = 1.0f, float circleSize = 1.0f, bool invertY = false, float startingPercentage = 100.0f);
	void updateTransformationMatrix();
	void isHidden(bool is);
	void draw();

	void fingersEnter(InputGestureDirectFingers::enterCursorArgs & a);
	void fingersUpdate(InputGestureDirectFingers::updateCursorArgs & a);
	void fingersTap(InputGestueTap::TapArgs & a);

private:
	FigureGraphic* base;
	FigureGraphic* sliderLine;
	FigureGraphic* sliderCircle;
	float sliderSize;
	float circleSize;
	bool invertY;
	float lastPercentage;
	float scaledHeight;
	ofVec3f basePoint;

	const float sliderLineHeight = 0.1f;
	const float sliderWidth = 0.05f;
};

#endif