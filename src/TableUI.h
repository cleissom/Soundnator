#ifndef _TABLEUI
#define _TABLEUI

#include "Figure.h"
#include "CollisionHelper.h"
#include "Polygon.h"
#include "FigureGraphic.hpp"
#include "InputGestureDirectFingers.hpp"
#include "InputGestureDirectObjects.hpp"
#include "InputGestureTap.hpp"
#include "InputGestureLongPush.hpp"

#include "Font.h"

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
	virtual void fingersLongPush(InputGestureLongPush::LongPushTrigerArgs & a) {};

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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class TableButton : public TableUIBase {

public:
	struct commomTableButtonArgs : public EventArgs
	{
		int id;
	};
	typedef commomTableButtonArgs TapButtonArgs;
	typedef commomTableButtonArgs LongPushButtonArgs;

	ofEvent<TapButtonArgs>		TapButton;
	ofEvent<LongPushButtonArgs> LongPushButton;

	TableButton(float angle = 0.0f, float distanceOffset = 0.05f, float size = 2.0f);
	void setImage(ofImage & image);
	void updateTransformationMatrix();
	void isHidden(bool);

	void fingersEnter(InputGestureDirectFingers::enterCursorArgs & a);
	void fingersTap(InputGestureTap::TapArgs & a);
	void fingersLongPush(InputGestureLongPush::LongPushTrigerArgs & a);

private:
	FigureGraphic* base;
	FigureGraphic* border;
	float buttonSize;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class TableSlider : public TableUIBase {
	struct commomTableSliderArgs : public EventArgs
	{
		int id;
		float value;
	};

public:
	typedef commomTableSliderArgs updateSliderArgs;

	ofEvent<updateSliderArgs> updateSlider;

	TableSlider(float angle = 0.0f, float distanceOffset = 0.075f, bool discreteSlider = false, float sliderMaxValue = 100.0f, float sliderMinValue = 0.0f, int id = 0, float sliderSize = 1.0f, float circleSize = 1.0f, bool invertY = false, bool tangent = true, bool showTopText = false, string bottomText = "");
	void updateTransformationMatrix();
	void isHidden(bool is);
	void draw();

	void setValue(float value) { this->lastValue = value; };
	void setMaxValue(float value) { this->sliderMaxValue = value; };

	void fingersEnter(InputGestureDirectFingers::enterCursorArgs & a);
	void fingersUpdate(InputGestureDirectFingers::updateCursorArgs & a);

private:
	FigureGraphic* base;
	FigureGraphic* sliderLine;
	FigureGraphic* sliderFillLine;
	FigureGraphic* sliderCircle;

	bool discreteSlider;
	float sliderMaxValue;
	float sliderMinValue;
	int id;
	float sliderSize;
	float circleSize;
	bool invertY;
	bool tangent;
	bool showTopText;
	string bottomText;

	bool hidden;
	ofMatrix4x4 sliderBottom;
	float scaledHeight;
	ofVec3f basePoint;
	float lastValue;

	Font font;

	const float sliderLineHeight = 0.1f;
	const float sliderWidth = 0.02f;
	const float linePolygonWidth = 0.005f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TableCell : public TableUIBase {
	struct commomTableCellArgs : public EventArgs
	{
		int id;
		bool selected;
	};

public:
	typedef commomTableCellArgs tapCellArgs;

	ofEvent<tapCellArgs> tapCell;

	TableCell(float angle = 0.0f, float distanceOffset = 0.075f, float openingAngle = 90, float thickness = 0.025f, bool clockwise = false, int id = 0, bool registerToEvents = true);
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TableSequencerCells : public TableUIBase {
	struct commomTableSequencerCellsArgs : public EventArgs
	{
		int id;
		bool state;
	};

public:
	typedef commomTableSequencerCellsArgs updateTableSequencerCellsArgs;

	ofEvent<updateTableSequencerCellsArgs> updateTableSequencerCells;

	TableSequencerCells(float angle = 0.0f, float distanceOffset = 0.075f, int cellsNum = 5, float openingAngle = 180, bool clockwise = true, float thickness = 0.04f);
	void updateTransformationMatrix();
	void isHidden(bool is);
	void updateSequencerCells(vector<bool>& vec);
	void setActiveCell(int num);

	void updateCallback(TableCell::tapCellArgs & a);

private:
	vector<TableCell*> cells;
	int cellsNum;

	const float gapAngle = 2.0f;
};




class TableSequencerSliders : public TableUIBase {
	struct commomTableSequencerSlidersArgs : public EventArgs
	{
		int id;
		float value;
	};

public:
	typedef commomTableSequencerSlidersArgs updateTableSequencerSlidersArgs;

	ofEvent<updateTableSequencerSlidersArgs> updateTableSequencerSliders;

	TableSequencerSliders(float angle = 0.0f, float distanceOffset = 0.075f, int cellsNum = 5, float openingAngle = 180, float maxValue = 100.0f, float minValue = 0.0f, bool clockwise = true);
	void updateTransformationMatrix();
	void isHidden(bool is);
	void updateSequencerSliders(vector<int>& vec);

	void updateCallback(TableSlider::updateSliderArgs & a);

private:
	vector<TableSlider*> sliders;
	int cellsNum;

	const float gapAngle = 2.0f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TableInfoCircle : public TableUIBase {
public:
	TableInfoCircle(float angle = 0.0f, float distanceOffset = 0.075f, float openingAngle = 180, bool clockwise = true, bool discrete = true, int cellsNum = 3, float maxValue = 100.0f, float minValue = 0.0f);

	void updateTransformationMatrix();
	void update();
	void draw();
	void isHidden(bool is);
	void setValue(float value) { this->lastValue = value; };

private:
	float openingAngle;
	bool clockwise;
	bool discrete;
	int cellsNum;
	float maxValue;
	float minValue;

	float lastValue = 75.0f;

	TableCell* continuousCell;
	bool fillCellIsHidden;

	vector<TableCell*> discreteCells;

	FigureGraphic* arrow;

	const float thickness = 0.01;
	const float arrowScale = 0.01f;
	const float gapAngle = 2.0f;
};

#endif