#include "SoundDispatcher.h"

SoundDispatcher::SoundDispatcher() {
	TableObjects.insert(std::make_pair(0, new Generator(0)));
	TableObjects.insert(std::make_pair(1, new Generator(1)));
	TableObjects.insert(std::make_pair(2, new Effect(2)));
	TableObjects.insert(std::make_pair(3, new Effect(3)));
	TableObjects.insert(std::make_pair(4, new Controller(4)));
	TableObjects.insert(std::make_pair(5, new Controller(5)));
	TableObjects.insert(std::make_pair(OUTPUT, new Output(OUTPUT)));

	registerEvent(InputGestureDirectFingers::I().enterCursor, &SoundDispatcher::addCursor, this);

	registerEvent(InputGestureDirectObjects::I().newObject, &SoundDispatcher::newObject, this);
	registerEvent(InputGestureDirectObjects::I().enterObject, &SoundDispatcher::enterObject, this);
	registerEvent(InputGestureDirectObjects::I().updateObject, &SoundDispatcher::updateObject, this);
	registerEvent(InputGestureDirectObjects::I().removeObject, &SoundDispatcher::exitObject, this);

	addOutputToTable();
}

void SoundDispatcher::addCursor(InputGestureDirectFingers::newCursorArgs & a) {
}

void SoundDispatcher::updateCursor(InputGestureDirectFingers::updateCursorArgs & a) {
}

void SoundDispatcher::addOutputToTable() {
	DirectObject* dobj = new DirectObject();
	dobj->s_id = -1;
	dobj->f_id = OUTPUT;
	dobj->setX(0.5f);
	dobj->setY(0.5f/3.0f);
	dobj->angle = 0;
	dobj->xspeed = 0;
	dobj->yspeed = 0;
	dobj->rspeed = 0;
	dobj->maccel = 0;
	dobj->raccel = 0;

	int id = dobj->f_id;
	TableObjects[id]->setDirectObject(dobj);
	ObjectsOnTable.insert(TableObjects[id]);
}

void SoundDispatcher::addObjectToTable(TableObject* obj) {
	if (obj) {
		if (ObjectsOnTable.find(obj) == ObjectsOnTable.end()) {
			ObjectsOnTable.insert(obj);
		}
	}
}

void SoundDispatcher::addObjectToTable(DirectObject* dobj) {
	int id = dobj->f_id;
	if (TableObjects[id]) {
		if (ObjectsOnTable.find(TableObjects[id]) == ObjectsOnTable.end()) {
			TableObjects[id]->setDirectObject(dobj);
			ObjectsOnTable.insert(TableObjects[id]);
		}
	}

}

void SoundDispatcher::removeObjectFromTable(DirectObject* dobj) {
	int id = dobj->f_id;
	if (TableObjects[id]) {
		if (ObjectsOnTable.find(TableObjects[id]) != ObjectsOnTable.end()) {
			TableObjects[id]->remove();
			ObjectsOnTable.erase(TableObjects[id]);
		}
	}

}


std::pair<TableObject*, TableObject*> SoundDispatcher::findShorterDistancesFrom(TableObject* obj) {
	float first = 10000;
	float second = 10001;
	std::pair<TableObject*, TableObject*> shorter(nullptr, nullptr);

	cout << "Finding Shorter Distance from " << obj->getId();

	for (TableObject* object : ObjectsOnTable) {
		cout << object->getId() << endl;
		if (object != obj) {
			float dist = obj->getDistanceTo(object);
			if (dist < first) {
				second = first;
				std::swap(shorter.first, shorter.second);
				first = dist;
				shorter.first = object;
			}
			else if (dist < second) {
				second = dist;
				shorter.second = object;
			}
		}
	}

	if (shorter.second) {
		cout << ". first: " << shorter.first->getId() << ". second: " << shorter.second->getId() << endl;
	}
	else if (shorter.first) {
		cout << ". first: " << shorter.first->getId() << endl;
	}

	return shorter;
}

void SoundDispatcher::processConnections(TableObject* object) {
	if (object) {
		cout << "processConnection " << object->getId() << endl;

		std::pair<TableObject*, TableObject*> shorter = findShorterDistancesFrom(object);

		if (shorter.first) {
			cout << "have first. ";
			if (shorter.second) {
				cout << "have second. ";
				shorter.second->isConnectedTo(shorter.first) ? (cout << "second is connected. ") : (cout << "second is not connected. ");

				object->canConnectTo(shorter.first) ? (cout << "can connect. ") : (cout << "can not connect. ");

				if (object->canConnectTo(shorter.first)) {

					if (shorter.second->isConnectedTo(shorter.first)) {
						auto distSecondToFirst = shorter.second->getDistanceTo(shorter.first);
						auto distObjToFirst = object->getDistanceTo(shorter.first);
						auto distObjToSecond = object->getDistanceTo(shorter.second);

						if ((distSecondToFirst > distObjToFirst) && (distObjToSecond < distSecondToFirst)) {
							cout << "distance. ";
							if (shorter.second->canConnectTo(object)) {
								shorter.second->connectTo(object);
							}
						}
					}

					if (shorter.first->haveConnection()) {
						object->connectTo(shorter.first);
					}
				}
			}
			else {
				cout << "not have second. ";
				if (object->canConnectTo(shorter.first)) {
					object->connectTo(shorter.first);
				}
			}
		}

		cout << endl;
	}

}



void SoundDispatcher::newObject(InputGestureDirectObjects::newObjectArgs& a) {
	cout << "New object" << endl;
	cout << " \n" << endl;
	cout << " \n" << endl;
	int id = a.object->f_id;
	addObjectToTable(a.object);
	processConnections(TableObjects[a.object->f_id]);
}

void SoundDispatcher::enterObject(InputGestureDirectObjects::enterObjectArgs& a) {
	cout << "Enter object" << endl;
}

void SoundDispatcher::updateObject(InputGestureDirectObjects::updateObjectArgs& a) {
	processConnections(TableObjects[a.object->f_id]);
}

void SoundDispatcher::exitObject(InputGestureDirectObjects::exitObjectArgs& a) {
	cout << "Exit object" << endl;
	removeObjectFromTable(a.object);
}