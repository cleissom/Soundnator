#include "TableObject.h"

pdsp::Engine engine;


TableObject::TableObject(int id, connectionType_t connection) : id(id), dobj(nullptr), connection(connection), followingObj(nullptr), precedingAudioObj(nullptr), precedingControlObj(nullptr) {
	//registerMyEvent(InputGestureDirectFingers::I().newCursor, &TableObject::addCursor, this);
	//registerMyEvent(InputGestureTap::I().Tap, &TableObject::Tap, this);

	registerEvent(InputGestureDirectObjects::I().updateObject, &TableObject::updateObject, this);

	addModuleInput("signal", input);
	addModuleInput("pitch", pitch_in);
	addModuleInput("trig", trig_in);
	addModuleOutput("signal", output);
	addModuleOutput("pitch", pitch_out);
	addModuleOutput("trig", trig_out);

	scope.set(bufferLen);
}

int TableObject::getId() {
	return this->id;
}

void TableObject::updateTurnMultiplier(float angle) {
	float derivative = angle - rawAngleLastValue;
	if (derivative > derivativeThreshold) turnsMultiplier--;
	else if (derivative < (-derivativeThreshold)) turnsMultiplier++;
	rawAngleLastValue = angle;
}

void TableObject::addCursor(InputGestureDirectFingers::newCursorArgs & a) {
}

void TableObject::Tap(InputGestureTap::TapArgs & a) {

}

void TableObject::updateObject(InputGestureDirectObjects::updateObjectArgs& a) {
	int id = a.object->f_id;
	if (id == this->id) {
		float rawAngle = a.object->angle;
		updateTurnMultiplier(rawAngle);
		float angle = turnsMultiplier * M_2PI + rawAngle;
		cout << angle << endl;
		updateAngleValue(angle);
	}
}





template <typename T>
bool TableObject::isObject(TableObject* node) {
	if (dynamic_cast<T*> (node)) {
		return true;
	}
	else {
		return false;
	}
}


TableObject** TableObject::getPrecedingObj(TableObject* obj) {
	return getPrecedingObj(obj, this->connection);
}

TableObject** TableObject::getPrecedingObj(TableObject* obj, connectionType_t connection) {
	switch (connection)
	{
	case AUDIO:
		return &obj->precedingAudioObj;
		break;
	case CONTROL:
		return &obj->precedingControlObj;
		break;
	default:
		break;
	}
	return nullptr;
}

void TableObject::makeConnectionTo(TableObject* obj) {
	switch (connection)
	{
	case AUDIO:
		*this >> *obj;
		break;
	case CONTROL:
		this->out("pitch") >> obj->in("pitch");
		this->out("trig") >> obj->in("trig");
		break;
	default:
		break;
	}
}

void TableObject::makeDisconnectionOut(TableObject* obj) {
	switch (connection)
	{
	case AUDIO:
		obj->out("signal").disconnectOut();
		break;
	case CONTROL:
		obj->out("pitch").disconnectOut();
		obj->out("trig").disconnectOut();
		break;
	default:
		break;
	}
}

void TableObject::makeDisconnectionIn(TableObject* obj) {
	switch (connection)
	{
	case AUDIO:
		obj->out("signal").disconnectIn();
		break;
	case CONTROL:
		obj->out("pitch").disconnectIn();
		obj->out("trig").disconnectIn();
		break;
	default:
		break;
	}
}



bool TableObject::isConnectedTo(TableObject* obj) {
	return obj == followingObj;
}

bool TableObject::canConnectTo(TableObject* obj) {
	if (isObject<Output>(this)) {
		return false;
	}
	else {

		TableObject** objPrecedingObj = getPrecedingObj(obj);

		if (isObject<Output>(obj) && objectIsConnectableToOutput()) {
			return true;
		}
		else if ((*objPrecedingObj == nullptr) && objectIsConnectableTo(obj)) {
			return true;
		}
		else {
			return false;
		}
	}
}





void TableObject::connectTo(TableObject* obj) {

	if (followingObj) {
		*getPrecedingObj(followingObj) = nullptr;
	}
	makeDisconnectionOut(this);

	makeConnectionTo(obj);

	followingObj = obj;

	*getPrecedingObj(obj) = this;
}





bool TableObject::haveConnection() {
	return (this->followingObj != nullptr);
}

void TableObject::remove() {

	if (precedingAudioObj && precedingControlObj && followingObj) {
		makeDisconnectionOut(precedingAudioObj);
		makeDisconnectionOut(precedingControlObj);

		precedingAudioObj->followingObj = nullptr;
		precedingControlObj->followingObj = nullptr;

		precedingAudioObj = nullptr;
		precedingControlObj = nullptr;
	}
	else if (precedingAudioObj && followingObj) {
		makeDisconnectionOut(precedingAudioObj);
		makeDisconnectionIn(followingObj);
		*precedingAudioObj >> *followingObj;
		precedingAudioObj->followingObj = followingObj;
		followingObj->precedingAudioObj = precedingAudioObj;
		precedingAudioObj = nullptr;
	}
	else if (precedingControlObj && followingObj) {
		makeDisconnectionOut(precedingControlObj);
		makeDisconnectionIn(followingObj);
		precedingControlObj->followingObj = nullptr;
		precedingControlObj = nullptr;
	}
	if (followingObj) {
		*getPrecedingObj(followingObj) = nullptr;
		followingObj = nullptr;
	}
	setDirectObject(nullptr);
	this->disconnectAll();
}


void TableObject::drawAudioWave() {
	ofPushMatrix();

	TableObject* nextObj = getFollowingObject();
	float dist = this->getDistanceTo(getFollowingObject());
	float yHalf = y / 2;
	ofTranslate(this->getDirectObject()->getX(), this->getDirectObject()->getY());
	ofRotateRad(this->getAngleTo(nextObj));
	ofTranslate(0, -yHalf);
	ofNoFill();
	ofSetLineWidth(1);

	int samples = dist / xMult;
	int bufferIndex = 1;
	float yMult = -yHalf;
	vector<float> buffer = scope.getBuffer();

	ofBeginShape();
	for (int xx = 0; xx < samples; xx++) {
		int index = xx * bufferIndex;
		float value = buffer[index];
		ofVertex(xx * xMult, yHalf + value * yMult);
	}
	ofEndShape(false);
	ofPopMatrix();
}

void TableObject::draw() {
	TableObject* nextObj = getFollowingObject();
	if (nextObj) {
		drawAudioWave();
	}

	objectDraw();
}


void TableObject::setDirectObject(DirectObject* dobj) {
	this->dobj = dobj;
}

DirectObject* TableObject::getDirectObject() {
	return dobj;
}

TableObject* TableObject::getFollowingObject() {
	return followingObj;
}

float TableObject::getDistanceTo(TableObject* obj) {
	return this->dobj->getDistance(obj->dobj);
}

float TableObject::getAngleTo(TableObject* obj) {
	return this->dobj->getAngle(obj->dobj);
}


void TableObject::setToScope(pdsp::Patchable& in) {
	in >> scope >> engine.blackhole();
}



Generator::Generator(int id, connectionType_t connection) : TableObject(id, connection) {
	patch();

	polygon.AddVertex(ofPoint(-0.025f, -0.05f));
	polygon.AddVertex(ofPoint(-0.025f, -0.075f));
	polygon.AddVertex(ofPoint(0.025f, -0.075f));
	polygon.AddVertex(ofPoint(0.025f, -0.05f));
	fg = new FigureGraphic(&polygon);
	fg->registerMyEvent(InputGestureDirectFingers::Instance().enterCursor, &Generator::enter, this);

	fg->color.r = ofRandom(0, 255);
	fg->color.g = ofRandom(0, 255);
	fg->color.b = ofRandom(0, 255);
	fg->color.a = 100;
	fg->transformation.setTranslation(0.5, 0.5, 0);
	fg->isHidden(true);

	fg->registerMyEvent(InputGestureTap::I().Tap, &Generator::Tap, this);

}

void Generator::enter(InputGestureDirectFingers::enterCursorArgs& a) {
	cout << "enter figure" << endl;

}

void Generator::update() {
	if (getDirectObject()) {
		fg->isHidden(false);
		fg->transformation.setTranslation(this->getDirectObject()->getX(), this->getDirectObject()->getY(), 0);
	}
	else {
		fg->isHidden(true);
	}
}

void Generator::patch() {

	//patchinga
	osc.out_triangle() >> amplifier >> output;
	env >> amplifier.in_mod();
	trig_in >> env;
	pitch_ctrl >> osc.in_pitch();
	pitch_ctrl.enableSmoothing(50.0f);
	amplifier.set(1.0f);
	trig_in.set(1.0f);
	this->setToScope(amplifier);


}

void Generator::updateAngleValue(float angle) {
	float pitch = ofMap(angle, 0, 5.0f * M_2PI, 36.0f, 96.0f);
	pitch_ctrl.set(pitch);
}

bool Generator::objectIsConnectableTo(TableObject* obj) {
	return isObject<Effect>(obj);
}

bool Generator::objectIsConnectableToOutput() {
	return true;
}

void Generator::Tap(InputGestureTap::TapArgs & a) {
	cout << "TAP" << endl;
	switch (choose % 2) {
	case 1:
		osc.out_triangle().disconnectOut();
		osc.out_saw() >> amplifier;
		break;
	case 0:
		osc.out_saw().disconnectOut();
		osc.out_triangle() >> amplifier;
		break;
	}
	choose++;
}

void Generator::objectDraw() {

}



Effect::Effect(int id, connectionType_t connection) : TableObject(id, connection) {
	patch();
}


void Effect::patch() {

	input >> filter >> amp >> output;
	this->setToScope(amp);
	cutoff_ctrl >> filter.in_cutoff();
	amp.set(1.0f);
}

void Effect::addCursor(InputGestureDirectFingers::newCursorArgs & a) {
}

void Effect::updateAngleValue(float angle) {
	float cutoff = ofMap(angle, 0, 2.0f * M_2PI, 48.0f, 96.0f);
	cutoff_ctrl.set(cutoff);
}

bool Effect::objectIsConnectableTo(TableObject* obj) {
	return isObject<Effect>(obj);
}

bool Effect::objectIsConnectableToOutput() {
	return true;
}




Controller::Controller(int id, connectionType_t connection) : TableObject(id, connection) {
	patch();
}



void Controller::patch() {
	pitch_ctrl >> osc;
	osc.out_pulse() >> amp >> trig_out;
	this->setToScope(amp);
	amp.set(1.0f);
}

void Controller::addCursor(InputGestureDirectFingers::newCursorArgs & a) {
}

void Controller::updateAngleValue(float angle) {
	float pitch = ofMap(angle, 0, 5.0f * M_2PI, 0.0f, 36.0f);
	pitch_ctrl.set(pitch);
}

bool Controller::objectIsConnectableTo(TableObject* obj) {
	return isObject<Generator>(obj) || isObject<Effect>(obj);
}

bool Controller::objectIsConnectableToOutput() {
	return false;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////


Output::Output(int id) : TableObject(id) {

	patch();
}


void Output::patch() {

	//------------SETUPS AND START AUDIO-------------
#ifdef PC
	engine.setDeviceID(1);
#else
	engine.setDeviceID(0);
#endif // PC

	engine.setup(44100, 512, 3);

	input >> engine.audio_out(0);
	input >> engine.audio_out(1);
}

void Output::draw() {
	ofFill();
	ofDrawCircle(getDirectObject()->getX(), getDirectObject()->getY(), 0.03f);
}

bool Output::haveConnection() {
	return true;
}

bool Output::objectIsConnectableTo(TableObject* obj) {
	return false;
}

bool Output::objectIsConnectableToOutput() {
	return false;
}
