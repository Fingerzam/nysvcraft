#include "Helpers.h"

using namespace BWAPI;
using namespace std;

Unit* helpers::getClosestEnemy(Unit* unit, set<Unit*> enemies) {		
	return getClosestUnitFrom(unit->getPosition(), enemies);
}

Unit* helpers::getClosestUnitFrom(Position &pos, set<Unit*> units) {
	Unit* closest = 0;
	double minDistance = numeric_limits<double>::infinity();
	
	for (set<BWAPI::Unit*>::const_iterator iter = units.begin(); iter != units.end(); iter++) {
		Unit* unit = *iter;
		double distance = unit->getPosition().getDistance(pos);
				
		if (distance < minDistance) {
			minDistance = distance;
			closest = unit;
		}
	}
	return closest;
}

set<Unit*> helpers::getAttackingAllies() {
	set<Unit*> allies = Broodwar->self()->getUnits();
	for (set<Unit*>::const_iterator iter = allies.begin(); iter != allies.end(); iter++) {
		Unit* ally = *iter;
		if (!isAttackingEnemy(ally)) {
			allies.erase(iter);
		}
	}
	return allies;
}

bool helpers::isAttackingEnemy(Unit* unit) {
	Unit* other = unit->getOrderTarget();
	return other && other->getPlayer() == Broodwar->enemy();
}