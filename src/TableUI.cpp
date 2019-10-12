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
	fg->registerMyEvent(InputGestureLongPush::I().LongPushTriger, &TableUIBase::fingersLongPush, this);
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
	polygon->SetTexture("1.png");
	base = new FigureGraphic(polygon);
	this->registerFingerEvents(base);
	base->color.r = 255;
	base->color.g = 255;
	base->color.b = 255;
	base->color.a = 200;
	base->hasAlpha(true);
	base->isHidden(true);
	
	polygon = new Figures::Polygon();
	drawCircleOnPolygon(polygon);
	border = new FigureGraphic(polygon);
	border->color.r = 255;
	border->color.g = 255;
	border->color.b = 255;
	border->color.a = 100;
	border->hasAlpha(true);
	border->canCollide(false);
	border->isHidden(true);
	border->setFill(false);

	updatePosition(0, 0);
};

void TableButton::updateTransformationMatrix() {
	/// local pivot -> direct order.  Rotate -> Translate -> Scale
	ofMatrix4x4 M;

	M.makeIdentityMatrix();
	M.glTranslate(this->getX(), this->getY(), 0);
	M.glRotate(this->getAngle(), 0, 0, 1);
	M.glTranslate(this->getDistanceOffset(), 0.0f, 0.0f);
	M.glScale(buttonSize * 0.01f, buttonSize * 0.01f, 1);
	M.glRotate(-90, 0, 0, 1);

	base->transformation = M;
	border->transformation = M;
}

void TableButton::fingersEnter(InputGestureDirectFingers::enterCursorArgs& a) {
	//cout << "fingers enter" << endl;
};

void TableButton::fingersTap(InputGestueTap::TapArgs& a) {
	cout << "tap" << endl;
	commomTableButtonArgs args;
	args.id = 1;
	ofNotifyEvent(TapButton, args);
};

void TableButton::fingersLongPush(InputGestureLongPush::LongPushTrigerArgs& a) {
	commomTableButtonArgs args;
	args.id = 1;
	ofNotifyEvent(LongPushButton, args);
};

void TableButton::isHidden(bool is) {
	base->isHidden(is);
	border->isHidden(is);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



TableSlider::TableSlider(float angle, float distanceOffset, bool discreteSlider, float sliderMaxValue, float sliderMinValue, int id, float sliderSize, float circleSize, bool invertY, bool tangent, bool showTopText, string bottomText) : TableUIBase(angle, distanceOffset), discreteSlider(discreteSlider), sliderMaxValue(sliderMaxValue), sliderMinValue(sliderMinValue), id(id), sliderSize(sliderSize), circleSize(circleSize), invertY(invertY), lastValue(sliderMaxValue), tangent(tangent), showTopText(showTopText), bottomText(bottomText)  {
	scaledHeight = sliderSize * sliderLineHeight;

	Figures::Polygon* polygon = new Figures::Polygon();

	polygon->AddVertex(ofPoint(-(sliderWidth / 2.0f), 0.0f));
	polygon->AddVertex(ofPoint((sliderWidth / 2.0f), 0.0f));
	polygon->AddVertex(ofPoint((sliderWidth / 2.0f), scaledHeight));
	polygon->AddVertex(ofPoint(-(sliderWidth / 2.0f), scaledHeight));
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
	polygon->AddVertex(ofPoint((linePolygonWidth / 2.0f), scaledHeight));
	polygon->AddVertex(ofPoint(-(linePolygonWidth / 2.0f), scaledHeight));
	sliderLine = new FigureGraphic(polygon);
	sliderLine->color.r = 255;
	sliderLine->color.g = 255;
	sliderLine->color.b = 255;
	sliderLine->color.a = 100;
	sliderLine->canCollide(false);
	sliderLine->hasAlpha(true);
	sliderLine->isHidden(true);
	
	polygon = new Figures::Polygon();
	polygon->AddVertex(ofPoint(-(linePolygonWidth / 2.0f), 0.0f));
	polygon->AddVertex(ofPoint((linePolygonWidth / 2.0f), 0.0f));
	polygon->AddVertex(ofPoint((linePolygonWidth / 2.0f), scaledHeight));
	polygon->AddVertex(ofPoint(-(linePolygonWidth / 2.0f), scaledHeight));
	sliderFillLine = new FigureGraphic(polygon);
	sliderFillLine->color.r = 255;
	sliderFillLine->color.g = 255;
	sliderFillLine->color.b = 255;
	sliderFillLine->color.a = 200;
	sliderFillLine->canCollide(false);
	sliderFillLine->hasAlpha(true);
	sliderFillLine->isHidden(true);

	

	updatePosition(0, 0);
};

void TableSlider::updateTransformationMatrix() {
	ofMatrix4x4 M;
	M.makeIdentityMatrix();

	M.glTranslate(this->getX(), this->getY(), 0);
	M.glRotate(this->getAngle(), 0, 0, 1);
	M.glTranslate(this->getDistanceOffset(), 0.0f, 0.0f);

	if (!tangent) {
		M.glRotate(-90.0f, 0, 0, 1);
	}

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
	sliderBottom = M;

	base->transformation = M;
	sliderLine->transformation = M;

	ofMatrix4x4 FillLineM = M;
	FillLineM.glScale(1, ((lastValue - sliderMinValue) / (sliderMaxValue - sliderMinValue)), 1);
	sliderFillLine->transformation = FillLineM;
	
	M.glTranslate(0, scaledHeight * ((lastValue - sliderMinValue) / (sliderMaxValue - sliderMinValue)), 0);
	M.glScale(circleSize * 0.01f, circleSize * 0.01f, 1);
	sliderCircle->transformation = M;
}

void TableSlider::isHidden(bool is) {
	this->hidden = is;
	sliderLine->isHidden(is);
	sliderFillLine->isHidden(is);
	sliderCircle->isHidden(is);

	base->canCollide(!is);
}

void TableSlider::draw() {
	if (!hidden) {
		ofPushMatrix();

		const float stringSpacing = 0.02f;

		ofMultMatrix(sliderBottom);

		if (showTopText) {
			stringstream buffer;
			buffer << int(lastValue) << endl;
			font.drawString(buffer.str(), true, 0, scaledHeight + stringSpacing);
		}

		if (!bottomText.empty()) {
			font.drawString(bottomText, true, 0, -stringSpacing, true);
		}

		ofPopMatrix();
	}
}

void TableSlider::fingersEnter(InputGestureDirectFingers::enterCursorArgs& a) {
	cout << "fingers enter slider" << endl;
};
void TableSlider::fingersUpdate(InputGestureDirectFingers::updateCursorArgs& a) {
	float percentage = ((a.finger->getDistance(basePoint.x, basePoint.y)) / scaledHeight);
	float percentageValue = sliderMinValue + (percentage * (sliderMaxValue - sliderMinValue));

	if (discreteSlider) {
		percentageValue = round(percentageValue);
	}


	cout << "fingers update: " << percentageValue << endl;

	updateSliderArgs args;
	args.id = id;
	args.value = percentageValue;
	ofNotifyEvent(updateSlider, args);

	lastValue = percentageValue;
};

void TableSlider::fingersTap(InputGestueTap::TapArgs& a) {
	cout << "tap slider" << endl;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TableCell::TableCell(float angle, float distanceOffset, float openingAngle, float thickness, bool clockwise, int id) : TableUIBase(angle, distanceOffset), openingAngle(openingAngle), clockwise(clockwise), id(id) {
	Figures::Polygon* polygon = new Figures::Polygon();

	// inner arc
	float step_value = M_PI / 80.0f;
	for (float i = 0.0f; i < ofDegToRad(openingAngle); i += step_value) {
		polygon->AddVertex(ofPoint(distanceOffset*cos(i), distanceOffset*sin(i)));
	}

	// outer arc
	for (float i = ofDegToRad(openingAngle); i > 0.0f; i -= step_value) {
		polygon->AddVertex(ofPoint((distanceOffset + thickness)*cos(i), (distanceOffset + thickness)*sin(i)));
	}

	base = new FigureGraphic(polygon);
	this->registerFingerEvents(base);
	base->isHidden(true);
	base->hasAlpha(true);


	updatePosition(0, 0);
};

void TableCell::updateTransformationMatrix() {
	ofMatrix4x4 M;
	M.makeIdentityMatrix();
	M.glTranslate(this->getX(), this->getY(), 0);
	M.glRotate(this->getAngle(), 0, 0, 1);

	if (clockwise) {
		M.glRotate(-openingAngle, 0, 0, 1);
	}
	base->transformation = M;


	if (active) {
		base->color.r = 255;
		base->color.b = 100;
		base->color.g = 100;
		base->color.a = 200;
	}
	else if (selected) {
		base->color.b = 255;
		base->color.b = 255;
		base->color.g = 255;
		base->color.a = 200;
	}
	else {
		base->color.b = 255;
		base->color.b = 255;
		base->color.g = 255;
		base->color.a = 100;
	}
}

void TableCell::isHidden(bool is) {
	base->isHidden(is);
	base->canCollide(!is);
}

void TableCell::fingersTap(InputGestueTap::TapArgs & a) {
	this->selected = not(selected);
	tapCellArgs args;
	args.id = this->id;
	args.selected = this->selected;
	ofNotifyEvent(tapCell, args);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TableSequencerCells::TableSequencerCells(float angle, float distanceOffset, int cellsNum, float openingAngle, bool clockwise, float thickness) : TableUIBase(angle, distanceOffset), cellsNum(cellsNum) {
	int gaps = cellsNum - 1;
	float totalCellOpeningAngle = openingAngle - float(gaps) * gapAngle;
	float cellOpeningAngle = totalCellOpeningAngle / float(cellsNum);

	float cellAngle;

	for (int i = 0; i <= cellsNum - 1; i++) {
		if (clockwise) {
			cellAngle = ofWrapDegrees(angle - ((float(i) * cellOpeningAngle) + (float(i) * gapAngle)));
		}
		else {
			cellAngle = ofWrapDegrees(angle + ((float(i) * cellOpeningAngle) + (float(i) * gapAngle)));
		}
		cells.push_back(new TableCell(cellAngle, distanceOffset, cellOpeningAngle, thickness, clockwise, i));
	}

	for (auto cell : cells) {
		registerEvent(cell->tapCell, &TableSequencerCells::updateCallback, this);
	}

}

void TableSequencerCells::updateTransformationMatrix() {
	for (auto cell : cells) {
		cell->updatePosition(this->getX(), this->getY());
	}
}

void TableSequencerCells::isHidden(bool is) {
	for (auto cell : cells) {
		cell->isHidden(is);
	}
}

void TableSequencerCells::updateSequencerCells(vector<bool>& vec) {
	for (size_t i = 0; i < cells.size(); i++) {
		cells[i]->isSelected(vec[i]);
	}
}

void TableSequencerCells::setActiveCell(int num) {
	for (int i = 0; i <= cellsNum - 1; i++) {
		cells[i]->isActive(i == num ? true : false);
	}
}

void TableSequencerCells::updateCallback(TableCell::tapCellArgs & a) {
	updateTableSequencerCellsArgs args;
	args.id = a.id;
	args.state = a.selected;
	ofNotifyEvent(updateTableSequencerCells, args);
}


TableSequencerSliders::TableSequencerSliders(float angle, float distanceOffset, int cellsNum, float openingAngle, float maxValue, float minValue, bool clockwise) : TableUIBase(angle, distanceOffset), cellsNum(cellsNum) {
	int gaps = cellsNum - 1;
	float totalCellOpeningAngle = openingAngle - float(gaps) * gapAngle;
	float cellOpeningAngle = totalCellOpeningAngle / float(cellsNum);

	float cellAngle;

	for (int i = 0; i <= cellsNum - 1; i++) {
		if (clockwise) {
			cellAngle = ofWrapDegrees(angle - ((float(i) * cellOpeningAngle) + (float(i) * gapAngle)));
		}
		else {
			cellAngle = ofWrapDegrees(angle + ((float(i) * cellOpeningAngle) + (float(i) * gapAngle)));
		}
		sliders.push_back(new TableSlider(cellAngle, distanceOffset, true, maxValue, minValue, i, 0.75f, 1.0f, false, false, true));
	}

	for (auto slider : sliders) {
		registerEvent(slider->updateSlider, &TableSequencerSliders::updateCallback, this);
	}

}

void TableSequencerSliders::updateTransformationMatrix() {
	for (auto slider : sliders) {
		slider->updatePosition(this->getX(), this->getY());
	}
}

void TableSequencerSliders::isHidden(bool is) {
	for (auto slider : sliders) {
		slider->isHidden(is);
	}
}

void TableSequencerSliders::updateSequencerSliders(vector<int>& vec) {
	for (size_t i = 0; i < sliders.size(); i++) {
		sliders[i]->setSliderValue(vec[i]);
	}
}

void TableSequencerSliders::updateCallback(TableSlider::updateSliderArgs & a) {
	updateTableSequencerSlidersArgs args;
	args.id = a.id;
	args.value = a.value;
	ofNotifyEvent(updateTableSequencerSliders, args);
}