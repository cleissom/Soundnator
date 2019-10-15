#include "TableObject.h"


TableObject::TableObject(int id, connectionType_t connection) : id(id), dobj(nullptr), connection(connection), followingObj(nullptr), precedingAudioObj(nullptr), precedingControlObj(nullptr) {
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
	float totalAngle = turnsMultiplier * M_2PI - angle;
	if ((derivative > derivativeThreshold) && (((turnsMultiplier*M_2PI) - angle) < angleMaxValue)) turnsMultiplier++;
	else if ((derivative < (-derivativeThreshold)) && (((2 * turnsMultiplier*M_2PI) - angle) > angleMinValue)) turnsMultiplier--;
	rawAngleLastValue = angle;
	cout << "turns: " << turnsMultiplier << endl;
}

void TableObject::updateObject(InputGestureDirectObjects::updateObjectArgs& a) {
	int id = a.object->f_id;
	if (id == this->id) {
		float rawAngle = a.object->angle;
		updateTurnMultiplier(rawAngle);
		float angle = turnsMultiplier * M_2PI - rawAngle;
		if (angle > angleMaxValue) angle = angleMaxValue;
		if (angle < angleMinValue) angle = angleMinValue;
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

void TableObject::updateTableUI(TableUIBase* ui, bool conditional) {
	if (this->getDirectObject()) {
		if (conditional) {
			ui->updatePosition(this->getDirectObject()->getX(), this->getDirectObject()->getY());
			ui->isHidden(false);
		}
		else {
			ui->isHidden(true);
		}
	}
	else {
		ui->isHidden(true);
	}
}

void TableObject::loadImg(ofImage& image, const std::string & dir) {
	image.load(dir);
	image.mirror(true, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Generator::Generator(int id, connectionType_t connection) : TableObject(id, connection) {};

bool Generator::objectIsConnectableTo(TableObject* obj) {
	return isObject<Effect>(obj);
}

bool Generator::objectIsConnectableToOutput() {
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Oscillator::Oscillator(int id) : Generator(id) {
	patch();

	angleMinValue = -TWO_PI;
	angleMaxValue = 2 * TWO_PI;

	actualMode = SINE;

	button = new TableButton(180.0f, 0.075f);
	slider = new TableSlider(270.0f, 0.06f);
	registerEvent(button->TapButton, &Oscillator::Tap, this);
	registerEvent(slider->updateSlider, &Oscillator::updateVolume, this);


	loadImg(sineImg, "1.png");
	loadImg(sawImg, "2.png");
	loadImg(pulseImg, "3.png");
}


void Oscillator::update() {

	if (actualModeChanged) {
		switch (actualMode) {
		case SINE:
			ampEnv.in_signal().disconnectIn();
			sine.out_sine() >> ampEnv;
			button->setImage(sineImg);
			break;
		case SAW:
			ampEnv.in_signal().disconnectIn();
			saw.out_saw() >> ampEnv;
			button->setImage(sawImg);
			break;
		case PULSE:
			ampEnv.in_signal().disconnectIn();
			pulse.out_pulse() >> ampEnv;
			button->setImage(pulseImg);
			break;
		default:
			break;
		}
		actualModeChanged = false;
	}

	updateTableUI(button);
	updateTableUI(slider);

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


void Oscillator::patch() {

	ampEnv >> amp * dB(-12.0f) >> output;
	trig_in >> env.in_trig();

	pitch_ctrl >> sine.in_pitch();
	pitch_ctrl >> saw.in_pitch();
	pitch_ctrl >> pulse.in_pitch();

	pitch_in >> sine.in_pitch();
	pitch_in >> saw.in_pitch();
	pitch_in >> pulse.in_pitch();
	pitch_ctrl.enableSmoothing(100.0f);

	amp.set(1.0f);
	ampEnv.set(1.0f);

	this->setToScope(amp);
}


void Oscillator::updateAngleValue(float angle) {
	//pitch_ctrl.set(akebono[ofClamp(angle, 0, akebono.size() - 1)]);
	pitch_ctrl.set(ofMap(angle, -TWO_PI, 2 * TWO_PI, 36, 72));
}

void Oscillator::updateVolume(TableSlider::updateSliderArgs& a) {
	cout << "update volume to: " << (a.value / 100.0f) << endl;
	amp.set(a.value / 100.0f);
}


void Oscillator::Tap(TableButton::TapButtonArgs& a) {

	switch (actualMode)
	{
	case SINE:
		actualMode = SAW;
		break;
	case SAW:
		actualMode = PULSE;
		break;
	case PULSE:
		actualMode = SINE;
		break;
	default:
		break;
	}
	actualModeChanged = true;
}



static vector<float> akebono{ 72.0f, 74.0f, 75.0f, 79.0f, 80.0f, 84.0f, 86.0f, 87.0f };


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sampler::Sampler(int id) : Generator(id) {
	patch();

	angleMinValue = -TWO_PI;
	angleMaxValue = 2 * TWO_PI;

	actualMode = SINE;

	button = new TableButton(180.0f, 0.075f);
	slider = new TableSlider(270.0f, 0.075f);
	registerEvent(button->TapButton, &Sampler::Tap, this);
	registerEvent(slider->updateSlider, &Sampler::updateVolume, this);

	loadImg(sineImg, "1.png");
	loadImg(sawImg, "2.png");
	loadImg(pulseImg, "3.png");

	violin.load("./data/a.wav");
	sampler.addSample(&violin);
}


void Sampler::update() {

	/*switch (actualMode) {
	case SINE:
		ampEnv.in_signal().disconnectIn();
		sine.out_sine() >> ampEnv;
		button->setImage(sineImg);
		break;
	case SAW:
		ampEnv.in_signal().disconnectIn();
		saw.out_saw() >> ampEnv;
		button->setImage(sawImg);
		break;
	case PULSE:
		ampEnv.in_signal().disconnectIn();
		pulse.out_pulse() >> ampEnv;
		button->setImage(pulseImg);
		break;
	default:
		break;
	}*/

	updateTableUI(button);
	updateTableUI(slider);

	/*if (connectionUpdated) {
		if (*getPrecedingObj(this, CONTROL)) {
			env >> ampEnv.in_mod();
			cout << "control" << endl;
		}
		else {
			env.disconnectOut();
			cout << "not control" << endl;
		}
		connectionUpdated = false;
	}*/
}


void Sampler::patch() {

	trig_in >> sampler >> amp * dB(-12.0f) >> output;
	trig_in >> env.set(0,200,200) >> amp.in_mod();
	

	//pitch_ctrl >> sampler.in_pitch();
	//pitch_in >> sampler.in_pitch();
	//-60.0 >> sampler.in_pitch();

	pitch_ctrl.enableSmoothing(100.0f);

	amp.set(1.0f);
	ampEnv.set(1.0f);

	this->setToScope(amp);
}


void Sampler::updateAngleValue(float angle) {
	//pitch_ctrl.set(akebono[ofClamp(angle, 0, akebono.size() - 1)]);
	pitch_ctrl.set(ofMap(angle, -TWO_PI, 2 * TWO_PI, 36, 72));
}

void Sampler::updateVolume(TableSlider::updateSliderArgs& a) {
	cout << "update volume to: " << (a.value / 100.0f) << endl;
	amp.set(a.value / 100.0f);
}


void Sampler::Tap(TableButton::TapButtonArgs& a) {

	switch (actualMode)
	{
	case SINE:
		actualMode = SAW;
		break;
	case SAW:
		actualMode = PULSE;
		break;
	case PULSE:
		actualMode = SINE;
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Effect::Effect(int id, connectionType_t connection) : TableObject(id, connection) {
}

bool Effect::objectIsConnectableTo(TableObject* obj) {
	return isObject<Effect>(obj);
}

bool Effect::objectIsConnectableToOutput() {
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Filter::Filter(int id) : Effect(id) {
	patch();

	angleMinValue = -TWO_PI;
	angleMaxValue = TWO_PI;

	button = new TableButton(180.0, 0.075);
	registerEvent(button->TapButton, &Filter::Tap, this);

	slider = new TableSlider(270.0f, 0.06f);
	registerEvent(slider->updateSlider, &Filter::updateSlider, this);

	info = new TableInfoCircle(150, 0.05, 150.0, true, false, 0, filterMaxValue, filterMinValue);
}


void Filter::patch() {

	input >> filter >> ampWet >>	amp;
	input			>> ampDry >>	amp;
									amp >> output;

	cutoff_ctrl >> filter.in_cutoff();
	cutoff_ctrl.enableSmoothing(50.0f);

	amp.set(1.0f);
	ampDry.set(0.0f);
	ampWet.set(1.0f);

	this->setToScope(amp);
}

void Filter::update() {

	if (actualModeChanged) {
		switch (actualMode)
		{
		case LOWPASS:
			pdsp::VAFilter::LowPass24 >> filter.in_mode();
			break;
		case HIGHPASS:
			pdsp::VAFilter::HighPass24 >> filter.in_mode();
			break;
		case BANDPASS:
			pdsp::VAFilter::BandPass24 >> filter.in_mode();
			break;
		default:
			break;
		}
		actualModeChanged = false;
	}

	updateTableUI(button);
	updateTableUI(info);
	updateTableUI(slider);
}

void Filter::updateAngleValue(float angle) {
	float cutoff = ofMap(angle, -TWO_PI, TWO_PI, filterMinValue, filterMaxValue);
	info->setValue(cutoff);
	cutoff_ctrl.set(cutoff);
}

void Filter::Tap(TableButton::TapButtonArgs& a) {

	switch (actualMode)
	{
	case LOWPASS:
		actualMode = HIGHPASS;
		break;
	case HIGHPASS:
		actualMode = BANDPASS;
		break;
	case BANDPASS:
		actualMode = LOWPASS;
		break;
	default:
		break;
	}
	actualModeChanged = true;
}

void Filter::updateSlider(TableSlider::updateSliderArgs& a) {
	float wetness = ofMap(a.value, 0.0, 100.0, 0, 1.0);
	ampWet.set(wetness);
	ampDry.set(1.0 - wetness);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Delay::Delay(int id) : Effect(id) {
	patch();

	angleMinValue = -TWO_PI;
	angleMaxValue = TWO_PI;

	button = new TableButton(180.0, 0.075);
	registerEvent(button->TapButton, &Delay::Tap, this);

	slider = new TableSlider(270.0f, 0.06f);
	registerEvent(slider->updateSlider, &Delay::updateSlider, this);

	info = new TableInfoCircle(150, 0.05, 150.0, true, false, 0, delayMaxValue, delayMinValue);
}


void Delay::patch() {

	input >> delay >> amp >> output;

	time_ctrl >> delay.in_time();
	feedback_ctrl >> delay.in_feedback();

	feedback_ctrl.enableSmoothing(50.0f);
	time_ctrl.enableSmoothing(50.0f);

	feedback_ctrl.set(1.0f);
	time_ctrl.set(0.0f);


	amp.set(1.0f);

	this->setToScope(amp);
}

void Delay::update() {

	/*if (actualModeChanged) {
		switch (actualMode)
		{
		case LOWPASS:
			pdsp::VAFilter::LowPass24 >> filter.in_mode();
			break;
		case HIGHPASS:
			pdsp::VAFilter::HighPass24 >> filter.in_mode();
			break;
		case BANDPASS:
			pdsp::VAFilter::BandPass24 >> filter.in_mode();
			break;
		default:
			break;
		}
		actualModeChanged = false;
	}*/

	updateTableUI(button);
	updateTableUI(info);
	updateTableUI(slider);
}

void Delay::updateAngleValue(float angle) {
	float time = ofMap(angle, -TWO_PI, TWO_PI, delayMinValue, delayMaxValue);
	info->setValue(time);
	time_ctrl.set(time);
}

void Delay::Tap(TableButton::TapButtonArgs& a) {

	switch (actualMode)
	{
	case LOWPASS:
		actualMode = HIGHPASS;
		break;
	case HIGHPASS:
		actualMode = BANDPASS;
		break;
	case BANDPASS:
		actualMode = LOWPASS;
		break;
	default:
		break;
	}
	actualModeChanged = true;
}

void Delay::updateSlider(TableSlider::updateSliderArgs& a) {
	float feedback = ofMap(a.value, 0.0, 100.0, 0, 1.0);
	feedback_ctrl.set(feedback);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Controller::Controller(int id, connectionType_t connection) : TableObject(id, connection) {
}


bool Controller::objectIsConnectableTo(TableObject* obj) {
	//return isObject<Generator>(obj) || isObject<Effect>(obj);
	return isObject<Generator>(obj);
}

bool Controller::objectIsConnectableToOutput() {
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sequencer::Sequencer(int id, int sequencerSection, connectionType_t connection) : Controller(id, connection), sequencerSection(sequencerSection) {
	patch();

	angleMaxValue = TWO_PI;
	angleMinValue = 0.0f;

	beats = vector<vector<bool>>(4, vector<bool>(beatsNum, false));
	pitches = vector<vector<int>>(4, vector<int>(beatsNum, 0));
	volumes = vector<vector<int>>(4, vector<int>(beatsNum, 100));

	actualSequence = 0;
	actualMode = SEQUENCER;

	auto& sequence = SoundEngine::I().getSection(sequencerSection).sequence(0);

	sequence.code = [&] {
		sequence.begin();

		int bars = sequence.bars;

		for (int i = 0; i <= 16 - 1; i++) {
			sequence.delay((i*bars) / 16.0f);
			sequence.out(0).bang(beats[actualSequence][i] ? float(volumes[actualSequence][i]) / 100.0f : 0.0f);
			sequence.out(1).bang(pitches[actualSequence][i]);
			sequence.delay(((i*bars) + 0.2f) / 16.0f).out(0).bang(0.0f);
		}

		sequence.end();
	};

	SoundEngine::I().getSection(sequencerSection).sequence(0).bars = 4.0f;

	SoundEngine::I().getSection(sequencerSection).launch(0);

	tableSequencerCells = new TableSequencerCells(0.0f, 0.07f, beatsNum, 320.0f);
	tableSequencerCells->updateSequencerCells(beats[actualSequence]);

	tableSequencerPitch = new TableSequencerSliders(-10.0f, 0.1f, beatsNum, 320.0f, 12, -12);
	tableSequencerPitch->updateSequencerSliders(pitches[actualSequence]);

	tableSequencerVolume = new TableSequencerSliders(-10.0f, 0.1f, beatsNum, 320.0f);
	tableSequencerVolume->updateSequencerSliders(volumes[actualSequence]);

	button = new TableButton(20.0f, 0.09f);
	tempoSlider = new TableSlider(-90.0f, 0.15f, true, 2.0f);
	info = new TableInfoCircle(0, 0.05, 160, true, true, 4, 4.0f, 0.0f);

	registerEvent(tableSequencerCells->updateTableSequencerCells, &Sequencer::updateTableSequencerCells, this);
	registerEvent(tableSequencerPitch->updateTableSequencerSliders, &Sequencer::updateTableSequencerPitch, this);
	registerEvent(tableSequencerVolume->updateTableSequencerSliders, &Sequencer::updateTableSequencerVolume, this);
	registerEvent(button->TapButton, &Sequencer::tapButton, this);
	registerEvent(button->LongPushButton, &Sequencer::longPushButton, this);
	registerEvent(tempoSlider->updateSlider, &Sequencer::updateTempoSlider, this);

}


void Sequencer::patch() {
	pitch_ctrl >> osc;
	//osc.out_pulse() >> amp >> trig_out;
	SoundEngine::I().getSection(sequencerSection).out_trig(0) >> trig_out;
	SoundEngine::I().getSection(sequencerSection).out_value(1) >> pitch_out;
	this->setToScope(amp);
	amp.set(1.0f);
}


void Sequencer::update() {

	switch (actualMode) {
	case SEQUENCER:
		showTableSequencerCells = true;
		showTableSequencerPitch = false;
		showTableSequencerVolume = false;
		break;
	case PITCH:
		showTableSequencerCells = false;
		showTableSequencerPitch = true;
		showTableSequencerVolume = false;
		break;
	case VOLUME:
		showTableSequencerCells = false;
		showTableSequencerPitch = false;
		showTableSequencerVolume = true;
		break;
	default:
		break;
	}

	updateTableUI(button);
	updateTableUI(tempoSlider, showSlider && showTableSequencerCells);
	updateTableUI(tableSequencerCells, showTableSequencerCells);
	updateTableUI(tableSequencerPitch, showTableSequencerPitch);
	updateTableUI(tableSequencerVolume, showTableSequencerVolume);
	updateTableUI(info);

	tableSequencerCells->setActiveCell(int(ofMap(SoundEngine::I().getSection(sequencerSection).sequence(0).meter_percent(), 0, 0.95, 0, 15)));
}

void Sequencer::updateAngleValue(float angle) {
	float sequenceValueContinuous = ofMap(angle, 0, TWO_PI, 0, 3.99, true);
	info->setValue(sequenceValueContinuous);
	int newSequenceValue = int(sequenceValueContinuous);
	if (newSequenceValue != actualSequence) {
		actualSequence = newSequenceValue;
		tableSequencerCells->updateSequencerCells(beats[actualSequence]);
		tableSequencerPitch->updateSequencerSliders(pitches[actualSequence]);
		tableSequencerVolume->updateSequencerSliders(volumes[actualSequence]);
	}
}


void Sequencer::updateTableSequencerCells(TableSequencerCells::updateTableSequencerCellsArgs & a) {
	beats[actualSequence][a.id] = a.state;
}
void Sequencer::updateTableSequencerPitch(TableSequencerSliders::updateTableSequencerSlidersArgs & a) {
	pitches[actualSequence][a.id] = a.value;
}
void Sequencer::updateTableSequencerVolume(TableSequencerSliders::updateTableSequencerSlidersArgs & a) {
	volumes[actualSequence][a.id] = a.value;
}

void Sequencer::tapButton(TableButton::TapButtonArgs & a) {
	switch (actualMode)
	{
	case SEQUENCER:
		actualMode = PITCH;
		break;
	case PITCH:
		actualMode = VOLUME;
		break;
	case VOLUME:
		actualMode = SEQUENCER;
		break;
	default:
		break;
	}
}

void Sequencer::longPushButton(TableButton::LongPushButtonArgs & a) {
	showSlider = not(showSlider);
}

void Sequencer::updateTempoSlider(TableSlider::updateSliderArgs & a) {
	float bars = float(1 << int(a.value));
	SoundEngine::I().getSection(sequencerSection).sequence(0).bars = bars;
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
