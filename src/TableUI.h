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
		float value;
	};

public:
	typedef commomTableSliderArgs updateSliderArgs;

	ofEvent<updateSliderArgs> updateSlider;

	TableSlider(float angle = 0.0f, float distanceOffset = 0.075f, bool discreteSlider = false, float sliderMaxValue = 100.0f, float sliderSize = 1.0f, float circleSize = 1.0f, bool invertY = false);
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
	float scaledHeight;
	ofVec3f basePoint;
	float lastValue;
	float sliderMaxValue;
	bool discreteSlider;

	const float sliderLineHeight = 0.1f;
	const float sliderWidth = 0.05f;
};

class TableCell : public TableUIBase {
	struct commomTableCellArgs : public EventArgs
	{
		int id;
		bool selected;
	};

public:
	typedef commomTableCellArgs tapCellArgs;

	ofEvent<tapCellArgs> tapCell;

	TableCell(float angle = 0.0f, float distanceOffset = 0.075f, float openingAngle = 90, float thickness = 0.025f, bool clockwise = false, int id = 0);
	void updateTransformationMatrix();
	void isHidden(bool is);

	void isSelected(bool is) { selected = is; };
	void isActive(bool is) { active = is; };

	void fingersTap(InputGestueTap::TapArgs & a);

private:
	int id;
	FigureGraphic* base;
	bool active = false;
	bool selected = false;
	float openingAngle;
	bool clockwise;
};


class TableSequencer : public TableUIBase {
	struct commomTableSequencerArgs : public EventArgs
	{
		int id;
		bool state;
	};

public:
	typedef commomTableSequencerArgs tapSequencerArgs;

	ofEvent<tapSequencerArgs> tapSequencer;

	TableSequencer(float angle = 0.0f, float distanceOffset = 0.075f, int cellsNum = 5, float openingAngle = 180, bool clockwise = false, float thickness = 0.04f);
	void updateTransformationMatrix();
	void isHidden(bool is);

	void tapCellSequencerCallback(TableCell::tapCellArgs & a);

	void setBeats(vector<bool>* beats) { this->beats = beats; }

	void updateSequencerCells(vector<bool>& vec);

private:
	vector<TableCell*> cells;
	vector<bool>* beats;
	const float gapAngle = 2.0f;
};

#endif