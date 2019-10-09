#include "TableObject.h"


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
	if (derivative > derivativeThreshold) turnsMultiplier++;
	else if (derivative < (-derivativeThreshold)) turnsMultiplier--;
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
		float angle = turnsMultiplier * M_2PI - rawAngle;
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
	obj->connectionUpdated = true;
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
	obj->connectionUpdated = true;
}

void TableObject::makeDisconnectionIn(TableObject* obj) {
	if (isObject<Output>(obj)) return;
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
	obj->connectionUpdated = true;
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

	ofSetColor(255, 255, 255, 255);

	TableObject* nextObj = getFollowingObject();
	float dist = this->getDistanceTo(getFollowingObject());
	float yHalf = y / 2;
	ofTranslate(this->getDirectObject()->getX(), this->getDirectObject()->getY());
	ofRotateRad(this->getAngleTo(nextObj));
	ofTranslate(0, -yHalf);
	ofNoFill();
	ofSetLineWidth(3);

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
	in >> scope >> SoundEngine::I().getEngine().blackhole();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Generator::Generator(int id, connectionType_t connection) : TableObject(id, connection) {
	patch();
	button = new TableButton(90.0f, 0.075f);
	slider = new TableSlider(237.0f, 0.1f);
	registerEvent(button->TapButton, &Generator::Tap, this);
	registerEvent(slider->updateSlider, &Generator::updateVolume, this);


}

void Generator::update() {
	if (getDirectObject()) {
		button->updatePosition(this->getDirectObject()->getX(), this->getDirectObject()->getY());
		slider->updatePosition(this->getDirectObject()->getX(), this->getDirectObject()->getY());
		button->isHidden(false);
		slider->isHidden(false);
	}
	else {
		button->isHidden(true);
		slider->isHidden(true);
	}

	if (connectionUpdated) {
		if (*getPrecedingObj(this, CONTROL)) {
			env >> ampEnv.in_mod();
			cout << "control" << endl;
		}
		else {
			env.disconnectOut();
			cout << "not control" << endl;
		}
		connectionUpdated = false;
	}
}


class Scale : public pdsp::Unit {
	Scale() {};

};

void Generator::patch() {

	//patchinga
	osc.out_pulse() >> ampEnv;
	ampEnv >> amp * dB(-12.0f) >> output;
	//env.set(0.0f, 50.0f, 1.0f, 100.0f) >> ampEnv.in_mod();
	trig_in >> env.in_trig();
	//1.0f >> env.in_trig();
	pitch_ctrl >> osc.in_pitch();
	pitch_in >> osc.in_pitch();
	pitch_ctrl.enableSmoothing(50.0f);

	//1.0f >> ampEnv.in_mod();

	amp.set(1.0f);
	//trig_in.set(1.0f);
	ampEnv.set(1.0f);

	this->setToScope(amp);
}

static vector<float> akebono{ 72.0f, 74.0f, 75.0f, 79.0f, 80.0f, 84.0f, 86.0f, 87.0f };

void Generator::updateAngleValue(float angle) {
	pitch_ctrl.set(akebono[ofClamp(angle, 0, akebono.size() - 1)]);
}

bool Generator::objectIsConnectableTo(TableObject* obj) {
	return isObject<Effect>(obj);
}

bool Generator::objectIsConnectableToOutput() {
	return true;
}

void Generator::updateVolume(TableSlider::updateSliderArgs& a) {
	cout << "update volume to: " << (a.value / 100.0f) << endl;
	amp.set(a.value / 100.0f);
}


void Generator::Tap(TableButton::TapButtonArgs& a) {
	switch (choose % 2) {
	case 1:
		osc.out_pulse().disconnectOut();
		osc.out_saw() >> ampEnv;
		break;
	case 0:
		osc.out_saw().disconnectOut();
		osc.out_pulse() >> ampEnv;
		break;
	}
	choose++;
}

void Generator::objectDraw() {

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Controller::Controller(int id, int sequencerSection, connectionType_t connection) : TableObject(id, connection),  sequencerSection(sequencerSection) {
	patch();


	beats = vector<bool>(beatsNum, true);


	actualSequence = 0;

	auto& seq = SoundEngine::I().getEngine().sequencer.sections[sequencerSection].sequence(0);

	seq.code = [&] {
		seq.begin();

		int bars = seq.bars;

		for (int i = 0; i <= 16 - 1; i++) {
			seq.delay((i*bars) / 16.0f);
			seq.out(0).bang(beats[i] ? 1.0f : 0.0f);
			seq.delay(((i*bars) + 0.2f) / 16.0f).out(0).bang(0.0f);
		}

		seq.end();
	};

	SoundEngine::I().getEngine().sequencer.sections[sequencerSection].sequence(0).bars = 4.0f;

	SoundEngine::I().getEngine().sequencer.sections[sequencerSection].launch(0);


	tableSequencer = new TableSequencer(0.0f, 0.075f, beatsNum, 320.0f, true);
	tableSequencer->updateSequencerCells(beats);
	tableSequencer->setBeats(&beats);

	registerEvent(tableSequencer->tapSequencer, &Controller::tapSequencer, this);

	button = new TableButton(20.0f, 0.09f);
	slider = new TableSlider(-90.0f, 0.15f, true, 2.0f);
	registerEvent(button->TapButton, &Controller::tapButton, this);
	registerEvent(slider->updateSlider, &Controller::updateSlider, this);
}



void Controller::patch() {
	pitch_ctrl >> osc;
	//osc.out_pulse() >> amp >> trig_out;
	SoundEngine::I().getEngine().sequencer.sections[sequencerSection].out_trig(0) >> trig_out;
	//SoundEngine::I().getEngine().sequencer.sections[0].out_value(1) >> pitch_out;
	this->setToScope(amp);
	amp.set(1.0f);
}

void Controller::update() {
	if (getDirectObject()) {
		tableSequencer->updatePosition(this->getDirectObject()->getX(), this->getDirectObject()->getY());
		button->updatePosition(this->getDirectObject()->getX(), this->getDirectObject()->getY());
		tableSequencer->isHidden(false);
		button->isHidden(false);
		
		if (showSlider) {
			slider->updatePosition(this->getDirectObject()->getX(), this->getDirectObject()->getY());
			slider->isHidden(false);
		}
		else {
			slider->isHidden(true);
		}


	}
	else {
		tableSequencer->isHidden(true);
		button->isHidden(true);
		slider->isHidden(true);
	}
}

void Controller::addCursor(InputGestureDirectFingers::newCursorArgs & a) {
}

void Controller::updateAngleValue(float angle) {
	//SoundEngine::I().getEngine().sequencer.sections[0].sequence(0).bars = (int)ofClamp(angle, 1, 4);
}

bool Controller::objectIsConnectableTo(TableObject* obj) {
	//return isObject<Generator>(obj) || isObject<Effect>(obj);
	return isObject<Generator>(obj);
}

bool Controller::objectIsConnectableToOutput() {
	return false;
}

void Controller::tapSequencer(TableSequencer::tapSequencerArgs & a) {
	cout << "Tap on: " << a.id << endl;
	beats[a.id] = a.state;
}

void Controller::tapButton(TableButton::TapButtonArgs & a) {
	showSlider = not(showSlider);
}

void Controller::updateSlider(TableSlider::updateSliderArgs & a) {
	float bars = float(1 << int(a.value));
	SoundEngine::I().getEngine().sequencer.sections[0].sequence(0).bars = bars;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////


Output::Output(int id) : TableObject(id) {

	patch();
}


void Output::patch() {

	//------------SETUPS AND START AUDIO-------------
#ifdef PC
	SoundEngine::I().getEngine().setDeviceID(1);
#else
	SoundEngine::I().getEngine().setDeviceID(0);
#endif // PC

	SoundEngine::I().getEngine().setup(44100, 512, 3);

	input >> SoundEngine::I().getEngine().audio_out(0);
	input >> SoundEngine::I().getEngine().audio_out(1);
}

void Output::draw() {
	ofFill();
	ofSetColor(255, 255, 255, 255);
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
