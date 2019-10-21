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
}

void TableObject::updateObject(InputGestureDirectObjects::updateObjectArgs& a) {
	int id = a.object->f_id;
	if (id == this->id) {
		float rawAngle = a.object->angle;
		updateTurnMultiplier(rawAngle);
		float angle = turnsMultiplier * M_2PI - rawAngle;
		if (angle > angleMaxValue) angle = angleMaxValue;
		if (angle < angleMinValue) angle = angleMinValue;
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

		followingObj->precedingAudioObj = nullptr;

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

		followingObj->precedingAudioObj = nullptr;

		precedingControlObj->followingObj = nullptr;
		precedingControlObj = nullptr;
	}
	else if (followingObj) {
		*getPrecedingObj(followingObj) = nullptr;
	}

	if (followingObj) {
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
	if (this->dobj && obj->dobj) {
		return this->dobj->getDistance(obj->dobj);
	}
	else {
		return 0.0f;
	}
}

float TableObject::getAngleTo(TableObject* obj) {
	if (this->dobj && obj->dobj) {
		return this->dobj->getAngle(obj->dobj);
	}
	else {
		return 0.0f;
	}
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

	actualModeChanged = true;

	actualMode = SINE;

	button = new TableButton(180.0f, 0.075f);
	slider = new TableSlider(270.0f, 0.07f);
	registerEvent(button->TapButton, &Oscillator::Tap, this);
	registerEvent(button->LongPushButton, &Oscillator::LongPush, this);
	registerEvent(slider->updateSlider, &Oscillator::updateVolume, this);

	ASlider = new TableSlider(270.0f, 0.09f, false, 100.0f, 0.0, 0, 1.0f, 1.0f, false, true, false, "A");
	registerEvent(ASlider->updateSlider, &Oscillator::updateAttack, this);
	ASlider->setValue(0.0);
	attack_ctrl.set(1.0f);

	RSlider = new TableSlider(270.0f, 0.11f, false, 100.0f, 0.0, 0, 1.0f, 1.0f, false, true, false, "R");
	registerEvent(RSlider->updateSlider, &Oscillator::updateRelease, this);
	RSlider->setValue(50.0);
	release_ctrl.set(releaseMax / 2.0f);

	loadImg(sineImg, "imgs/sine.png");
	loadImg(sawImg, "imgs/saw.png");
	loadImg(pulseImg, "imgs/pulse.png");
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
	updateTableUI(ASlider, showAttackSlider);
	updateTableUI(RSlider, showReleaseSlider);

	if (connectionUpdated) {
		if (*getPrecedingObj(this, CONTROL)) {
			env >> ampEnv.in_mod();
		}
		else {
			env.disconnectOut();
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

	attack_ctrl >> env.in_attack();
	release_ctrl >> env.in_release();

	amp.set(1.0f);
	ampEnv.set(1.0f);

	this->setToScope(amp);
}


void Oscillator::updateAngleValue(float angle) {
	//pitch_ctrl.set(akebono[ofClamp(angle, 0, akebono.size() - 1)]);
	pitch_ctrl.set(ofMap(angle, -TWO_PI, 2 * TWO_PI, 48, 84));
}

void Oscillator::updateVolume(TableSlider::updateSliderArgs& a) {
	amp.set(a.value / 100.0f);
}

void Oscillator::updateAttack(TableSlider::updateSliderArgs& a) {
	float value = ofMap(a.value, 0, 100, attackMin, attackMax);
	attack_ctrl.set(value);
}

void Oscillator::updateRelease(TableSlider::updateSliderArgs& a) {
	float value = ofMap(a.value, 0, 100, releaseMin, releaseMax);
	release_ctrl.set(value);
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

void Oscillator::LongPush(TableButton::LongPushButtonArgs& a) {

	showAttackSlider = !showAttackSlider;
	showReleaseSlider = !showReleaseSlider;
}



static vector<float> akebono{ 72.0f, 74.0f, 75.0f, 79.0f, 80.0f, 84.0f, 86.0f, 87.0f };


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

pdsp::SampleBuffer* newSample(string dir) {
	pdsp::SampleBuffer* buffer = new pdsp::SampleBuffer();
	buffer->load(dir);
	return buffer;
}



vector<pdsp::SampleBuffer*> getSampleBuffers(string folder) {
	string path = "samples/" + folder;
	ofDirectory dir(path);
	dir.allowExt("wav");
	dir.listDir();
	vector<pdsp::SampleBuffer*> kicks;
	for (int i = 0; i < dir.size(); i++) {
		ofLogNotice(dir.getPath(i));
		kicks.push_back(newSample(dir.getAbsolutePath() + "/" + dir.getName(i)));
	}
	return kicks;
}

Sampler::Sampler(int id) : Generator(id) {
	patch();

	angleMinValue = -TWO_PI;
	angleMaxValue = 2 * TWO_PI;

	actualInstrument = KICK;

	button = new TableButton(180.0f, 0.075f);
	slider = new TableSlider(270.0f, 0.07f);
	registerEvent(button->TapButton, &Sampler::Tap, this);
	registerEvent(button->LongPushButton, &Sampler::LongPush, this);
	registerEvent(slider->updateSlider, &Sampler::updateVolume, this);

	ASlider = new TableSlider(270.0f, 0.09f, false, 100.0f, 0.0, 0, 1.0f, 1.0f, false, true, false, "A");
	registerEvent(ASlider->updateSlider, &Sampler::updateAttack, this);
	ASlider->setValue(0.0);

	RSlider = new TableSlider(270.0f, 0.11f, false, 100.0f, 0.0, 0, 1.0f, 1.0f, false, true, false, "R");
	registerEvent(RSlider->updateSlider, &Sampler::updateRelease, this);
	RSlider->setValue(100.0);



	loadImg(kickImg, "imgs/kick.png");
	loadImg(clapImg, "imgs/clap.png");
	loadImg(hatImg, "imgs/hat.png");
	loadImg(snareImg, "imgs/drum.png");
	loadImg(melodicImg, "imgs/sampler.png");


	samples[KICK] = getSampleBuffers("kick");
	samples[CLAP] = getSampleBuffers("clap");
	samples[HAT] = getSampleBuffers("hat");
	samples[SNARE] = getSampleBuffers("snare");
	samples[MELODIC] = getSampleBuffers("melodic");


	for (auto samplesVec : samples) {
		for (auto sample : samplesVec.second) {
			sampler.addSample(sample);
		}
	}



	instrumentSlider = new TableSlider(90.0f, 0.06f, true, samples[KICK].size() - 1, 0.0f, 0, 1, 1, true, true, true);
	registerEvent(instrumentSlider->updateSlider, &Sampler::instrumentSliderUpdate, this);
	instrumentSlider->setValue(0.0f);

	for (auto instrument : lastInstrumentValue) {
		instrument.second = 0;
	}

	actualInstrumentChanged = true;
}


void Sampler::update() {

	if (actualInstrumentChanged) {
		switch (actualInstrument)
		{
		case KICK:
			button->setImage(kickImg);
			break;
		case CLAP:
			button->setImage(clapImg);
			break;
		case HAT:
			button->setImage(hatImg);
			break;
		case SNARE:
			button->setImage(snareImg);
			break;
		case MELODIC:
			button->setImage(melodicImg);
			break;
		default:
			break;
		}
		actualInstrumentChanged = false;
	}



	updateTableUI(button);
	updateTableUI(slider);
	updateTableUI(ASlider, showAttackSlider);
	updateTableUI(RSlider, showReleaseSlider);
	updateTableUI(instrumentSlider, showInstrumentSlider);
}


void Sampler::patch() {

	trig_in >> sampler >> amp * dB(-12.0f) >> output;
	trig_in >> env >> amp.in_mod();

	pitch_ctrl >> sampler.in_pitch();
	pitch_in >> sampler.in_pitch();
	-60.0 >> sampler.in_pitch();
	//-72.0 >> sampler.in_pitch();


	select_ctrl >> sampler.in_select();

	attack_ctrl >> env.in_attack();
	release_ctrl >> env.in_release();

	releaseMax >> env.in_release();

	pitch_ctrl.enableSmoothing(100.0f);

	amp.set(1.0f);
	ampEnv.set(1.0f);

	this->setToScope(amp);
}


void Sampler::updateAngleValue(float angle) {
	//pitch_ctrl.set(akebono[ofClamp(angle, 0, akebono.size() - 1)]);
	pitch_ctrl.set(int(ofMap(angle, -TWO_PI, 2 * TWO_PI, 36, 72)));
}

void Sampler::updateVolume(TableSlider::updateSliderArgs& a) {
	amp.set(a.value / 100.0f);
}



int Sampler::sumInstrumentSize(InstrumentType v) {
	return samples[v].size();
}

template<typename... Types>
int Sampler::sumInstrumentSize(InstrumentType v, Types&&... others) {
	return samples[v].size() + sumInstrumentSize(others...);
}

void Sampler::changeInstrument(InstrumentType actual, InstrumentType next) {
	lastInstrumentValue[actual] = select_ctrl.get() - select_ctrl_offset;


	switch (actual)
	{
	case KICK:
		select_ctrl_offset = sumInstrumentSize(KICK);
		break;
	case CLAP:
		select_ctrl_offset = sumInstrumentSize(KICK, CLAP);
		break;
	case HAT:
		select_ctrl_offset = sumInstrumentSize(KICK, CLAP, HAT);
		break;
	case SNARE:
		select_ctrl_offset = sumInstrumentSize(KICK, CLAP, HAT, SNARE);
		break;
	case MELODIC:
		select_ctrl_offset = 0;
		break;
	default:
		break;
	}

	actualInstrument = next;
	instrumentSlider->setMaxValue(samples[next].size() - 1);
	select_ctrl.set(lastInstrumentValue[next] + select_ctrl_offset);
	instrumentSlider->setValue(lastInstrumentValue[next]);
	actualInstrumentChanged = true;
}

void Sampler::Tap(TableButton::TapButtonArgs& a) {

	switch (actualInstrument)
	{
	case KICK:
		changeInstrument(actualInstrument, CLAP);
		break;

	case CLAP:
		changeInstrument(actualInstrument, HAT);
		break;

	case HAT:
		changeInstrument(actualInstrument, SNARE);
		break;

	case SNARE:
		changeInstrument(actualInstrument, MELODIC);
		break;

	case MELODIC:
		changeInstrument(actualInstrument, KICK);
		break;
	default:
		break;
	}
}

void Sampler::updateAttack(TableSlider::updateSliderArgs& a) {
	float value = ofMap(a.value, 0, 100, attackMin, attackMax);
	attack_ctrl.set(value);
}

void Sampler::updateRelease(TableSlider::updateSliderArgs& a) {
	float value = ofMap(a.value, 0, 100, releaseMin, releaseMax);
	release_ctrl.set(value);
}

void Sampler::instrumentSliderUpdate(TableSlider::updateSliderArgs& a) {
	float value = a.value;
	select_ctrl.set(float(select_ctrl_offset) + value);
}

void Sampler::LongPush(TableButton::LongPushButtonArgs& a) {

	showAttackSlider = !showAttackSlider;
	showReleaseSlider = !showReleaseSlider;
	showInstrumentSlider = !showInstrumentSlider;
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

	slider = new TableSlider(270.0f, 0.07f);
	registerEvent(slider->updateSlider, &Filter::updateSlider, this);

	info = new TableInfoCircle(150, 0.05, 150.0, true, false, 0, filterMaxValue, filterMinValue);

	loadImg(lowpassImg, "imgs/lowpass.png");
	loadImg(highpassImg, "imgs/highpass.png");
	loadImg(bandpassImg, "imgs/bandpass.png");

	actualModeChanged = true;
}


void Filter::patch() {

	input >> filter >> ampWet >> amp;
	input >> ampDry >> amp;
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
			button->setImage(lowpassImg);
			break;
		case HIGHPASS:
			pdsp::VAFilter::HighPass24 >> filter.in_mode();
			button->setImage(highpassImg);
			break;
		case BANDPASS:
			pdsp::VAFilter::BandPass24 >> filter.in_mode();
			button->setImage(bandpassImg);
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
	registerEvent(button->LongPushButton, &Delay::LongPush, this);

	slider = new TableSlider(270.0f, 0.07f);
	registerEvent(slider->updateSlider, &Delay::updateSlider, this);
	slider->setValue(50.0);

	info = new TableInfoCircle(150, 0.05, 150.0, true, false, 0, delayMaxValue, delayMinValue);

	actualModeChanged = true;

	loadImg(feedbackImg, "imgs/feedback.png");
	loadImg(reverbImg, "imgs/reverb.png");
}


void Delay::patch() {

	input * dB(6.0f) >> reverb;
	input >> delay;

	node >> amp >> output;

	time_ctrl >> delay.in_time();
	feedback_ctrl >> delay.in_feedback();

	time_ctrl >> reverb.in_time();


	feedback_ctrl.enableSmoothing(500.0f);
	time_ctrl.enableSmoothing(200.0f);

	feedback_ctrl.set(0.5f);
	time_ctrl.set(0.0f);


	amp.set(1.0f);

	this->setToScope(amp);
}

void Delay::update() {

	if (actualModeChanged) {
		switch (actualMode)
		{
		case FEEDBACK_DELAY:
			node.disconnectIn();
			delay >> node;
			reverb >> SoundEngine::I().getEngine().blackhole();
			button->setImage(feedbackImg);
			break;
		case REVERB:
			node.disconnectIn();
			reverb >> node;
			feedback_ctrl >> reverb.in_density();
			button->setImage(reverbImg);
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

void Delay::updateAngleValue(float angle) {
	float time = ofMap(angle, -TWO_PI, TWO_PI, delayMinValue, delayMaxValue);
	info->setValue(time);
	time_ctrl.set(time);
}

void Delay::Tap(TableButton::TapButtonArgs& a) {

	switch (actualMode)
	{
	case FEEDBACK_DELAY:
		actualMode = REVERB;
		break;
	case REVERB:
		actualMode = FEEDBACK_DELAY;
		break;
	default:
		break;
	}
	actualModeChanged = true;
}

void Delay::LongPush(TableButton::LongPushButtonArgs& a) {

	/*showAttackSlider = !showAttackSlider;
	showReleaseSlider = !showReleaseSlider;*/
}

void Delay::updateSlider(TableSlider::updateSliderArgs& a) {
	float feedback = ofMap(a.value, 0.0, 100.0, 0, 0.8f);
	feedback_ctrl.set(feedback);
}

bool Delay::objectIsConnectableTo(TableObject* obj) {
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Chorus::Chorus(int id) : Effect(id) {
	patch();

	angleMinValue = -TWO_PI;
	angleMaxValue = TWO_PI;

	button = new TableButton(180.0, 0.075);
	registerEvent(button->TapButton, &Chorus::Tap, this);

	slider = new TableSlider(270.0f, 0.07f);
	registerEvent(slider->updateSlider, &Chorus::updateSlider, this);

	speedSlider = new TableSlider(270.0f, 0.09f);
	registerEvent(speedSlider->updateSlider, &Chorus::updateSpeedSlider, this);
	speed_ctrl.set(speedMaxValue / 2.0);
	speedSlider->setValue(50);

	delaySlider = new TableSlider(270.0f, 0.11f);
	registerEvent(delaySlider->updateSlider, &Chorus::updateDelaySlider, this);
	delay_ctrl.set(delayMaxValue / 2.0);
	delaySlider->setValue(50);

	info = new TableInfoCircle(150, 0.05, 150.0, true, false, 0, depthMaxValue, depthMinValue);

	loadImg(chorusImg, "imgs/chorus.png");
	button->setImage(chorusImg);

	showSpeedSlider = false;
	showDelaySlider = false;
}


void Chorus::patch() {

	input >> chorus >> ampWet >> amp;
	input >> ampDry >> amp;
	amp >> output;

	delay_ctrl >> chorus.in_delay();
	delay_ctrl.enableSmoothing(200.0f);

	speed_ctrl >> chorus.in_speed();
	speed_ctrl.enableSmoothing(200.0f);

	depth_ctrl >> chorus.in_depth();
	depth_ctrl.enableSmoothing(200.0f);

	amp.set(1.0f);
	ampDry.set(0.0f);
	ampWet.set(1.0f);

	this->setToScope(amp);
}

void Chorus::update() {
	updateTableUI(button);
	updateTableUI(info);
	updateTableUI(slider);
	updateTableUI(speedSlider, showSpeedSlider);
	updateTableUI(delaySlider, showDelaySlider);
}

void Chorus::updateAngleValue(float angle) {
	float value = ofMap(angle, -TWO_PI, TWO_PI, depthMinValue, depthMaxValue);
	info->setValue(value);
	depth_ctrl.set(value);
}

void Chorus::Tap(TableButton::TapButtonArgs& a) {
	showSpeedSlider = !showSpeedSlider;
	showDelaySlider = !showDelaySlider;
}

void Chorus::updateSlider(TableSlider::updateSliderArgs& a) {
	float wetness = ofMap(a.value, 0.0, 100.0, 0, 1.0);
	ampWet.set(wetness);
	ampDry.set(1.0 - wetness);
}

void Chorus::updateSpeedSlider(TableSlider::updateSliderArgs& a) {
	float value = ofMap(a.value, 0.0, 100.0, speedMinValue, speedMaxValue);
	speed_ctrl.set(value);
}

void Chorus::updateDelaySlider(TableSlider::updateSliderArgs& a) {
	float value = ofMap(a.value, 0.0, 100.0, delayMinValue, delayMaxValue);
	delay_ctrl.set(value);
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
			sequence.delay(((i*bars) + (pulseWidth*bars)) / 16.0f).out(0).bang(0.0f);
		}

		sequence.end();
	};

	SoundEngine::I().getSection(sequencerSection).sequence(0).bars = 2.0f;

	SoundEngine::I().getSection(sequencerSection).launch(0);

	tableSequencerCells = new TableSequencerCells(0.0f, 0.07f, beatsNum, 320.0f);
	tableSequencerCells->updateSequencerCells(beats[actualSequence]);

	tableSequencerPitch = new TableSequencerSliders(-10.0f, 0.1f, beatsNum, 320.0f, 12, -12, true, true);
	tableSequencerPitch->updateSequencerSliders(pitches[actualSequence]);

	tableSequencerVolume = new TableSequencerSliders(-10.0f, 0.1f, beatsNum, 320.0f);
	tableSequencerVolume->updateSequencerSliders(volumes[actualSequence]);

	button = new TableButton(20.0f, 0.09f);
	tempoSlider = new TableSlider(-90.0f, 0.13f, true, 2.0f);
	tempoSlider->setValue(1.0f);
	widthSlider = new TableSlider(90.0f, 0.13f, false, 100, 0, 0, 1, 1, true, true, false, "W");
	pulseWidth = 0.9;
	info = new TableInfoCircle(0, 0.05, 160, true, true, 4, 4.0f, 0.0f);

	registerEvent(tableSequencerCells->updateTableSequencerCells, &Sequencer::updateTableSequencerCells, this);
	registerEvent(tableSequencerPitch->updateTableSequencerSliders, &Sequencer::updateTableSequencerPitch, this);
	registerEvent(tableSequencerVolume->updateTableSequencerSliders, &Sequencer::updateTableSequencerVolume, this);
	registerEvent(button->TapButton, &Sequencer::tapButton, this);
	registerEvent(button->LongPushButton, &Sequencer::longPushButton, this);
	registerEvent(tempoSlider->updateSlider, &Sequencer::updateTempoSlider, this);
	registerEvent(widthSlider->updateSlider, &Sequencer::updateWidthSlider, this);

	loadImg(sequencerImg, "imgs/sequencer.png");
	loadImg(pitchImg, "imgs/pitch.png");
	loadImg(volumeImg, "imgs/volume.png");

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

	if (actualModeChanged) {
		switch (actualMode) {
		case SEQUENCER:
			showTableSequencerCells = true;
			showTableSequencerPitch = false;
			showTableSequencerVolume = false;
			button->setImage(sequencerImg);
			break;
		case PITCH:
			showTableSequencerCells = false;
			showTableSequencerPitch = true;
			showTableSequencerVolume = false;
			button->setImage(pitchImg);
			break;
		case VOLUME:
			showTableSequencerCells = false;
			showTableSequencerPitch = false;
			showTableSequencerVolume = true;
			button->setImage(volumeImg);
			break;
		default:
			break;
		}
		actualModeChanged = false;
	}


	updateTableUI(button);
	updateTableUI(tempoSlider, showSlider && showTableSequencerCells);
	updateTableUI(widthSlider, showSlider && showTableSequencerCells);
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
	actualModeChanged = true;
}

void Sequencer::longPushButton(TableButton::LongPushButtonArgs & a) {
	showSlider = not(showSlider);
}

void Sequencer::updateTempoSlider(TableSlider::updateSliderArgs & a) {
	float bars = float(1 << int(a.value));
	SoundEngine::I().getSection(sequencerSection).sequence(0).bars = bars;
}

void Sequencer::updateWidthSlider(TableSlider::updateSliderArgs & a) {
	pulseWidth = ofMap(a.value, 0, 100.0f, 0.0f, 0.9f);

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
