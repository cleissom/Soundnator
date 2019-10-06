#include "TableUI.h"

void drawCircleOnPolygon(Figures::Polygon* polygon, float step = 30.0f) {
	float step_value = TWO_PI / step;
	for (float i = 0.0f; i < TWO_PI; i += step_value) {
		polygon->AddVertex(ofPoint(cos(i), sin(i)));
	}
}


TableUIBase::TableUIBase(float angle, float distanceOffset) : angle(angle), distanceOffset(distanceOffset) {
};

void TableUIBase::registerFingerEvents(FigureGraphic* fg) {
	fg->registerMyEvent(InputGestureDirectFingers::I().enterCursor, &TableUIBase::fingersEnter, this);
	fg->registerMyEvent(InputGestureDirectFingers::I().updateCursor, &TableUIBase::fingersUpdate, this);
	fg->registerMyEvent(InputGestureTap::I().Tap, &TableUIBase::fingersTap, this);
};

void TableUIBase::updatePosition(float x, float y) {
	this->x_center = x;
	this->y_center = y;
	this->updateTransformationMatrix();
}

void TableUIBase::updateAngle(float angle) {
	this->angle = angle;
	this->updateTransformationMatrix();
}

void TableUIBase::updateDistanceOffset(float distance) {
	this->distanceOffset = distance;
	this->updateTransformationMatrix();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TableButton::TableButton(float angle, float distanceOffset, float size) : TableUIBase(angle, distanceOffset), buttonSize(size) {

	Figures::Polygon* polygon = new Figures::Polygon();
	drawCircleOnPolygon(polygon);
	base = new FigureGraphic(polygon);
	this->registerFingerEvents(base);
	base->color.r = ofRandom(0, 255);
	base->color.g = ofRandom(0, 255);
	base->color.b = ofRandom(0, 255);
	base->color.a = 100;
	base->isHidden(true);
};

void TableButton::updateTransformationMatrix() {
	/// local pivot -> direct order.  Rotate -> Translate -> Scale
	base->transformation.makeIdentityMatrix();

	base->transformation.glTranslate(this->getX(), this->getY(), 0);
	base->transformation.glRotate(this->getAngle(), 0, 0, 1);
	base->transformation.glTranslate(this->getDistanceOffset(), 0.0f, 0.0f);
	base->transformation.glScale(buttonSize * 0.01f, buttonSize * 0.01f, 1);
}

void TableButton::fingersEnter(InputGestureDirectFingers::enterCursorArgs& a) {
	//cout << "fingers enter" << endl;
};

void TableButton::fingersTap(InputGestueTap::TapArgs& a) {
	//cout << "tap" << endl;
	commomTableButtonArgs args;
	args.id = 1;
	ofNotifyEvent(TapButton, args);
};

void TableButton::isHidden(bool is) {
	base->isHidden(is);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



TableSlider::TableSlider(float angle, float distanceOffset, float sliderSize, float circleSize, bool invertY) : TableUIBase(angle, distanceOffset), sliderSize(sliderSize), circleSize(circleSize), invertY(invertY) {
	scaledHeight = sliderSize * sliderLineHeight;

	Figures::Polygon* polygon = new Figures::Polygon();

	polygon->AddVertex(ofPoint(-(sliderWidth/2.0f), 0.0f));
	polygon->AddVertex(ofPoint((sliderWidth / 2.0f), 0.0f));
	polygon->AddVertex(ofPoint((sliderWidth / 2.0f), sliderLineHeight));
	polygon->AddVertex(ofPoint(-(sliderWidth / 2.0f), sliderLineHeight));
	base = new FigureGraphic(polygon);
	this->registerFingerEvents(base);
	base->isHidden(true);

	polygon = new Figures::Polygon();
	drawCircleOnPolygon(polygon);
	sliderCircle = new FigureGraphic(polygon);
	sliderCircle->color.r = 255;
	sliderCircle->color.g = 255;
	sliderCircle->color.b = 255;
	sliderCircle->color.a = 255;
	sliderCircle->canCollide(false);
	sliderCircle->isHidden(true);

	float linePolygonWidth = 0.005f;
	polygon = new Figures::Polygon();
	polygon->AddVertex(ofPoint(-(linePolygonWidth / 2.0f), 0.0f));
	polygon->AddVertex(ofPoint((linePolygonWidth / 2.0f), 0.0f));
	polygon->AddVertex(ofPoint((linePolygonWidth / 2.0f), sliderLineHeight));
	polygon->AddVertex(ofPoint(-(linePolygonWidth / 2.0f), sliderLineHeight));
	sliderLine = new FigureGraphic(polygon);
	sliderLine->color.r = 255;
	sliderLine->color.g = 255;
	sliderLine->color.b = 255;
	sliderLine->color.a = 100;
	sliderLine->canCollide(false);
	sliderLine->hasAlpha(true);
	sliderLine->isHidden(true);
};

void TableSlider::updateTransformationMatrix() {
	ofMatrix4x4 M;
	M.makeIdentityMatrix();

	M.glTranslate(this->getX(), this->getY(), 0);
	M.glRotate(this->getAngle(), 0, 0, 1);
	M.glTranslate(this->getDistanceOffset(), 0.0f, 0.0f);

	
	float halfSize = scaledHeight / 2.0f;

	if (invertY) {
		M.glTranslate(0.0f, halfSize, 0.0f);
		M.glRotate(180, 0, 0, 1);
	}
	else {
		M.glTranslate(0.0f, -halfSize, 0.0f);
	}

	basePoint = ofVec3f(0, 0, 0);
	basePoint = basePoint * M;

	base->transformation = M;
	sliderLine->transformation = M;


	M.glTranslate(0, scaledHeight * (lastPercentage / 100.0f), 0);
	M.glScale(circleSize * 0.01f, circleSize * 0.01f, 1);
	sliderCircle->transformation = M;
}

void TableSlider::isHidden(bool is){
	sliderLine->isHidden(is);
	sliderCircle->isHidden(is);
}

void TableSlider::draw() {
}

void TableSlider::fingersEnter(InputGestureDirectFingers::enterCursorArgs& a) {
	cout << "fingers enter slider" << endl;
};
void TableSlider::fingersUpdate(InputGestureDirectFingers::updateCursorArgs& a) {
	cout << "fingers update slider" << endl;
	float percentage = ((a.finger->getDistance(basePoint.x, basePoint.y)) / scaledHeight)*100.0f;

	cout << "fingers update: " << percentage << endl;

	updateSliderArgs args;
	args.percentage = percentage;
	ofNotifyEvent(updateSlider, args);

	lastPercentage = percentage;
};

void TableSlider::fingersTap(InputGestueTap::TapArgs& a) {
	cout << "tap slider" << endl;
};