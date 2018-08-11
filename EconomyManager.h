#pragma once



class ZZerg;

class EconomyManager
{
	ZZerg & bot;

	public:
	EconomyManager(ZZerg &bot);
	~EconomyManager();
	

	void EconomyManagerAll();		//all submodules combined


	void EconomyOptimizer();
	void QueenInjectManager();
	void SetWorkerWaypoint();
	void CreepSpread();
	void UpgradesManager();

	void WorkerIdleManager(const sc2::Unit* unit);   //On unit idle (only for workers)

	const sc2::Unit * FindNearestUnitToPos(sc2::Point2D pos,sc2::UNIT_TYPEID unit_type);
	const sc2::Unit * FindNearestMineralNode(const sc2::Unit* unit) const;
	const sc2::Unit * FindNearestBase(const sc2::Unit* unit) const;
	const sc2::Unit * FindNearestWorker(const sc2::Unit* unit);
	const sc2::Unit * FindNearestMineralCarrier(const sc2::Unit* unit);
	const sc2::Unit * FindNonOptimalMiningLoc();
	bool InRangeOfTumor(sc2::Point2D point);

	float GetFinishedBasesCount() const;
	float GetWorkerCount() const;
	float GetActiveGasCount() const;
	bool UnitIsInRadius(const sc2::Unit* unit, const sc2::Unit* unit2);
	bool AlreadyTraining(const sc2::Units &units,sc2::ABILITY_ID ability);
	size_t AlreadyTrainingCount(const sc2::Units units, sc2::ABILITY_ID ability) const;
	bool WorkersNeeded(sc2::Units & units, sc2::Units & units2);
	
	int drones_per_ext;
	bool creep_tumors_placed = false;

	


	
};

