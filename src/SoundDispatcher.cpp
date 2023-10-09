#include "SoundDispatcher.h"

#include "SoundEngine.h"

SoundDispatcher::SoundDispatcher() {
    SoundEngine::I().getEngine().sequencer.init(4, 4, 100);

    TableObjects.insert(std::make_pair(0, new Oscillator(0)));
    TableObjects.insert(std::make_pair(1, new Oscillator(1)));
    TableObjects.insert(std::make_pair(2, new Filter(2)));
    TableObjects.insert(std::make_pair(3, new Filter(3)));
    TableObjects.insert(std::make_pair(4, new Sequencer(4, 0)));
    TableObjects.insert(std::make_pair(5, new Sequencer(5, 1)));
    TableObjects.insert(std::make_pair(6, new Delay(6)));
    TableObjects.insert(std::make_pair(7, new Sampler(7)));
    TableObjects.insert(std::make_pair(8, new Sampler(8)));
    TableObjects.insert(std::make_pair(9, new Sampler(9)));
    TableObjects.insert(std::make_pair(10, new Sequencer(10, 2)));
    TableObjects.insert(std::make_pair(11, new Delay(11)));
    TableObjects.insert(std::make_pair(12, new Chorus(12)));
    TableObjects.insert(std::make_pair(13, new Chorus(13)));
    TableObjects.insert(std::make_pair(14, new Sequencer(14, 3)));

    TableObjects.insert(std::make_pair(OUTPUT, new Output(OUTPUT)));

    registerEvent(InputGestureDirectFingers::I().enterCursor, &SoundDispatcher::addCursor, this);

    registerEvent(InputGestureDirectObjects::I().newObject, &SoundDispatcher::newObject, this);
    registerEvent(InputGestureDirectObjects::I().enterObject, &SoundDispatcher::enterObject, this);
    registerEvent(InputGestureDirectObjects::I().updateObject, &SoundDispatcher::updateObject, this);
    registerEvent(InputGestureDirectObjects::I().removeObject, &SoundDispatcher::exitObject, this);

    addOutputToTable();
}

void SoundDispatcher::addCursor(InputGestureDirectFingers::newCursorArgs &a) {}

void SoundDispatcher::updateCursor(InputGestureDirectFingers::updateCursorArgs &a) {}

void SoundDispatcher::addOutputToTable() {
    DirectObject *dobj = new DirectObject();
    dobj->s_id = -1;
    dobj->f_id = OUTPUT;
    dobj->setX(0.5f);
    dobj->setY(0.5f / 3.0f);
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

void SoundDispatcher::addObjectToTable(TableObject *obj) {
    if (obj) {
        if (ObjectsOnTable.find(obj) == ObjectsOnTable.end()) {
            ObjectsOnTable.insert(obj);
        }
    }
}

void SoundDispatcher::addObjectToTable(DirectObject *dobj) {
    int id = dobj->f_id;
    if (TableObjects[id]) {
        if (ObjectsOnTable.find(TableObjects[id]) == ObjectsOnTable.end()) {
            TableObjects[id]->setDirectObject(dobj);
            ObjectsOnTable.insert(TableObjects[id]);
        }
    }
}

void SoundDispatcher::removeObjectFromTable(DirectObject *dobj) {
    int id = dobj->f_id;
    if (TableObjects[id]) {
        if (ObjectsOnTable.find(TableObjects[id]) != ObjectsOnTable.end()) {
            TableObjects[id]->remove();
            ObjectsOnTable.erase(TableObjects[id]);
        }
    }
}

std::pair<TableObject *, TableObject *> SoundDispatcher::findShorterDistancesFrom(TableObject *obj) {
    float first = 10000;
    float second = 10001;
    std::pair<TableObject *, TableObject *> shorter(nullptr, nullptr);

    for (TableObject *object : ObjectsOnTable) {
        if (object != obj) {
            float dist = obj->getDistanceTo(object);
            if (dist < first) {
                second = first;
                std::swap(shorter.first, shorter.second);
                first = dist;
                shorter.first = object;
            } else if (dist < second) {
                second = dist;
                shorter.second = object;
            }
        }
    }

    return shorter;
}

void SoundDispatcher::processConnections(TableObject *object) {
    if (object) {
        std::pair<TableObject *, TableObject *> shorter = findShorterDistancesFrom(object);
        if (shorter.first) {
            if (shorter.second) {
                if (object->canConnectTo(shorter.first)) {
                    if (shorter.second->isConnectedTo(shorter.first)) {
                        auto distSecondToFirst = shorter.second->getDistanceTo(shorter.first);
                        auto distObjToFirst = object->getDistanceTo(shorter.first);
                        auto distObjToSecond = object->getDistanceTo(shorter.second);

                        if ((distSecondToFirst > distObjToFirst) && (distObjToSecond < distSecondToFirst)) {
                            if (shorter.second->canConnectTo(object)) {
                                shorter.second->connectTo(object);
                            }
                        }
                    }

                    if (shorter.first->haveConnection()) {
                        object->connectTo(shorter.first);
                    }
                }
            } else {
                if (object->canConnectTo(shorter.first)) {
                    object->connectTo(shorter.first);
                }
            }
        }
    }
}

void SoundDispatcher::newObject(InputGestureDirectObjects::newObjectArgs &a) {
    /*cout << "New object" << endl;
    cout << " \n" << endl;
    cout << " \n" << endl;*/
    int id = a.object->f_id;
    addObjectToTable(a.object);
    processConnections(TableObjects[a.object->f_id]);
}

void SoundDispatcher::enterObject(InputGestureDirectObjects::enterObjectArgs &a) {}

void SoundDispatcher::updateObject(InputGestureDirectObjects::updateObjectArgs &a) {
    processConnections(TableObjects[a.object->f_id]);
}

void SoundDispatcher::exitObject(InputGestureDirectObjects::exitObjectArgs &a) { removeObjectFromTable(a.object); }