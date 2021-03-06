#include "ExpansionManager.h"

using namespace BWTA;

ExpansionManager::ExpansionManager(Arbitrator::Arbitrator<Unit*, double>* arbitrator, BuildManager* buildManager,
								   BaseManager* baseManager, DefenseManager* defenseManager, WorkerManager* workerManager) {
	this->arbitrator = arbitrator;
	this->buildManager = buildManager;
	this->baseManager = baseManager;
	this->defenseManager = defenseManager;
	this->workerManager = workerManager;
	this->expansionCount = 0;
	this->lastExpanded = 0;
	this->expansionInterval = 1000;
	this->expansionStep = 1.8;
	this->occupiedBases = set<BaseLocation*>();
	this->enemyBases = set<Unit*>();
	this->occupiedBases.insert(getStartLocation(Broodwar->self()));
	this->initialized = false;
	initEnemyBuildings();
}

ExpansionManager::~ExpansionManager(void) {
}

void ExpansionManager::initEnemyBuildings() {
	foreach (Region* region, BWTA::getRegions()) {
		enemymyBuildings[region] = new set<Unit*>();
	}
}

void ExpansionManager::onOffer(set<Unit*> units) {
}

void ExpansionManager::onRevoke(Unit* unit, double bid) {
}

void ExpansionManager::update() {
	if (!analyzed) {
		initEnemyBuildings();
	}
	if (shouldExpand()) {
		expand();
	}
}

bool ExpansionManager::shouldExpand() {
	double workersPerMineral = ((double)workerManager->getWorkerCount())/mineralCount();
	Broodwar->drawTextScreen(450, 25, "workersPerMinerals: %.2f/%.2f", workersPerMineral, expansionStep);
	int framesFromLastExpand = Broodwar->getFrameCount() - lastExpanded;
	return (workersPerMineral >= expansionStep || Broodwar->self()->minerals() >= 3000) && framesFromLastExpand > 2500;
}

int ExpansionManager::mineralCount() {
	int sum = 0;
	foreach (Base* base, interestingBases()) {
		sum += base->getMinerals().size();
	}
	return sum;
}

void ExpansionManager::expand() {
	BaseLocation* buildLocation = expansionLocation();
	if (!buildLocation)
		return; //no valid expansions left
	Broodwar->printf("Expand #%d", expansionCount);
	Base* expansion = baseManager->expand(buildLocation, 100);
	expansionCount++;
	expansionStep -= 0.08;
	lastExpanded = Broodwar->getFrameCount();
	expansionInterval /= 2;
	defenseManager->onExpand(expansion);
}

/*
 * finds nearest free BaseLocation, return NULL if none available
 */
BaseLocation* ExpansionManager::expansionLocation() {
	BWTA::BaseLocation* location = NULL;
	double minDist = -1;
	BWTA::BaseLocation* home = BWTA::getStartLocation(BWAPI::Broodwar->self());
	foreach (BWTA::BaseLocation* base, BWTA::getBaseLocations()) {
		if (occupied(base))
			continue;
		double distance = home->getGroundDistance(base);
		if (minDist == -1 || distance < minDist) {
			minDist = distance;
			location = base;
		}
	}
	return location;
}

bool ExpansionManager::hasResources(BaseLocation* base) {
	return base->getMinerals().size() > 0;
}

void ExpansionManager::expansionFailed(Unit* base) {
	this->defenseManager->onFailedExpansion(base);
}

bool ExpansionManager::occupied(BaseLocation* base) {
	return occupiedBases.find(base) != occupiedBases.end();
}

string ExpansionManager::getName() const {
	return "Expansion Manager";
}

string ExpansionManager::getShortName() const {
	return "Expansion";
}

//TODO: should maybe keep track of nexuses instead of BaseLocations,
// since there could possibly be multiple bases close to a single BaseLocations
void ExpansionManager::onUnitShow(Unit* unit) {
	assert(unit);
	if (unit->getType() == UnitTypes::Protoss_Nexus) {
		occupiedBases.insert(baseLocation(unit));
		if (unit->getPlayer() == Broodwar->enemy()) {
			enemyBases.insert(unit);
		}
	}
	if (unit->getType().isBuilding()) {
		buildingSeen(unit);
	}
}

void ExpansionManager::buildingSeen(Unit* building) {
	if (building->getPlayer() != Broodwar->enemy())
		return;
	Region* region = BWTA::getRegion(building->getTilePosition());
	if (enemyBuildings[region]->find(building) != enemyBuildings[region]->end()) {
		enemyBuildings[region]->insert(building);
	}
}

void ExpansionManager::onUnitDestroy(Unit* unit) {
	assert(unit);
	if (unit->getType() == UnitTypes::Protoss_Nexus) {
		occupiedBases.erase(baseLocation(unit));
		if (unit->getPlayer() == Broodwar->enemy()) {
			enemyBases.erase(unit);
		}
	}
	if (unit->getType()->isBuilding()) {
		buildingDestroyed();
	}
}

void ExpansionManager::buildingDestroyed(Unit* building) {
	if (building->getPlayer() != Broodwar->enemy())
		return;
	Region* region = BWTA::getRegion(building->getTileLocation());
	enemyBuildings[region]->erase(building);
}

bool ExpansionManager::hasEnemyBuildings(Region* region) {
	return enemyBuildings[region]->size() > 0;
}

set<Region*> ExpansionManager::getEnemyRegions() {
	set<Region*> regions = set<Region*>();
	foreach (Region* region, BWTA::getRegions()) {
		if (hasEnemyBuildings(region)
			regions.insert(region);
	}
	return regions;
}

set<Unit*> ExpansionManager::getEnemyBuildingsOn(Region* region) {
	return set<Unit*>(*(enemyBuildings[region]));
}

BaseLocation* ExpansionManager::baseLocation(Unit* unit) {
	Region* reg = BWTA::getRegion(unit->getTilePosition());
	set<BaseLocation*> baseLocations = reg->getBaseLocations();
	if (baseLocations.size() == 0) {
		return BWTA::getNearestBaseLocation(unit->getTilePosition());
	} else {
		return *(baseLocations.begin());
	}
}

//bases that are either active or being constructed
set<Base*> ExpansionManager::interestingBases() {
	set<Base*> bases = set<Base*>();
	foreach (Base* base, this->baseManager->getAllBases()) {
		if (base->isActive() || base->isBeingConstructed())
			bases.insert(base);
	}
	return bases;
}

set<Unit*> ExpansionManager::getEnemyBases() {
	return set<Unit*>(this->enemyBases);
}
