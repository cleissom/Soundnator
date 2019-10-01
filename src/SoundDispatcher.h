#ifndef _SOUNDDISPATCHER
#define _SOUNDDISPATCHER

#include "SoundnatorApp.h"

const int OUTPUT = 216;

class SoundDispatcher : public EventClient {

	

	std::map<int, TableObject*> TableObjects;
	std::set<TableObject* > ObjectsOnTable;

public:
	SoundDispatcher();

	void addCursor(InputGestureDirectFingers::newCursorArgs & a);
	void updateCursor(InputGestureDirectFingers::updateCursorArgs & a);

	void addOutputToTable();
	void addObjectToTable(TableObject* obj);
	void addObjectToTable(DirectObject* dobj);
	void removeObjectFromTable(DirectObject* dobj);
	void processConnections(TableObject* object);
	std::pair<TableObject*, TableObject*> findShorterDistancesFrom(TableObject* obj);

	void newObject(InputGestureDirectObjects::newObjectArgs& a);
	void enterObject(InputGestureDirectObjects::enterObjectArgs& a);
	void updateObject(InputGestureDirectObjects::updateObjectArgs& a);
	void exitObject(InputGestureDirectObjects::exitObjectArgs& a);
};

#endif