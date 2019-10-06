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
	cout << "fingers enter" << endl;
};

void TableButton::fingersTap(InputGestueTap::TapArgs& a) {
	cout << "tap" << endl;
	commomTableButtonArgs args;
	args.id = 1;
	ofNotifyEvent(TapButton, args);
};

void TableButton::isHidden(bool is) {
	base->isHidden(is);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



TableSlider::TableSlider() {
	Figures::Polygon* polygon = new Figures::Polygon();

	polygon->AddVertex(ofPoint(0.05f, 0.1f));
	polygon->AddVertex(ofPoint(0.05f, -0.1f));
	polygon->AddVertex(ofPoint(0.1f, -0.1f));
	polygon->AddVertex(ofPoint(0.1f, 0.1f));

	base = new FigureGraphic(polygon);
	this->registerFingerEvents(base);


	polygon = new Figures::Polygon();
	drawCircleOnPolygon(polygon);
	sliderCircle = new FigureGraphic(polygon);


	sliderCircle->color.r = ofRandom(0, 255);
	sliderCircle->color.g = ofRandom(0, 255);
	sliderCircle->color.b = ofRandom(0, 255);
	sliderCircle->color.a = 100;
	sliderCircle->isHidden(true);
};

void TableSlider::draw() {
	/*ofPushMatrix();
	ofTranslate(this->get, this->y, 0);
	ofTranslate(0.075f, 0.0f, 0);
	ofDrawLine(0.0f, 0.1f, 0.0f, -0.1f);
	ofSetLineWidth(3);
	ofPopMatrix();*/
}

void TableSlider::fingersEnter(InputGestureDirectFingers::enterCursorArgs& a) {
	//cout << "fingers enter" << endl;
};
void TableSlider::fingersUpdate(InputGestureDirectFingers::updateCursorArgs& a) {
	/*float size = 0.2f;
	float min = this->y - size/2.0f;
	float percentage = ((a.finger->getY() - min) / size)*100.0f;

	cout << "fingers update: " << percentage << endl;

	updateSliderArgs args;
	args.percentage = percentage;
	ofNotifyEvent(updateSlider, args);*/
};

void TableSlider::fingersTap(InputGestueTap::TapArgs& a) {
	cout << "a" << endl;
};