#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include <limits>
#include <cmath>

/*States are:
	flee			- for running away from enemy
	fight			- kicking ass
	formation		- when moving in formation finding enemy or doing something
*/
enum State { flee, fight, formation };

struct UnitData {
	State state;
	int fleeCounter;
	int group; // Which group the unit belongs?
	int attackCounter;
};

namespace helpers {
	extern std::map<BWAPI::UnitType, unsigned int> fleeThreshold;
	static char* unitTypes[5] = { "Protoss Zealot", "Protoss Dragoon", "Protoss Probe", "Zerg Mutalisk", "Zerg Scourge" };
	static int typeCount = 5;
	BWAPI::Unit* getClosestUnitFrom(BWAPI::Position &pos, std::set<BWAPI::Unit*> units);
	BWAPI::Unit* getClosestEnemy(BWAPI::Unit* unit, std::set<BWAPI::Unit*> enemies);
	std::set<BWAPI::Unit*> getAttackingAllies();
	bool isAttackingEnemy(BWAPI::Unit* unit);
	bool contains(BWAPI::UnitType type, std::set<BWAPI::Unit*> units);

	// Returns the angle between two vectors
	// Interprets given positions as origo-centered vectors
	double angleBetween(BWAPI::Position, BWAPI::Position);

	double angleBetween(BWAPI::Position, double x, double y);

	BWAPI::Position vecFromAngle(double angle, int tiles = 1);

	int deadUnitCount();
	int killedUnitCount();
	void initializeFleeThresholds();

	bool shouldFlee(BWAPI::Unit* unit, std::set<BWAPI::Unit*> attackers);
	int getFleeDuration(BWAPI::Unit* unit, std::set<BWAPI::Unit*>* attackers);
	BWAPI::Unit* getLolEnemy(BWAPI::Unit* unit, std::set<BWAPI::Unit*> enemies);
}
