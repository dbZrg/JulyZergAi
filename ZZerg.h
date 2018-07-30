#pragma once
#include <iostream>

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "EconomyManager.h"
#include "BuildingManager.h"
#include "ArmyManagment.h"
#include "EnemyInfo.h"
#include "ProductionManager.h"


class ZZerg : public sc2::Agent
{
	BuildingManager bld;
	EconomyManager eco;
	ArmyManagment army;
	EnemyInfo enemy_info;
	ProductionManager production;
	

public:
	ZZerg();
	~ZZerg();

	/// \brief Called when the game starts
	void OnGameStart() final;

	/// \brief Called each update
	void OnStep() final;

	/// \brief Called when the game ends
	void OnGameEnd() final;

	/// \brief Called when unit is idle
	void OnUnitIdle(const sc2::Unit* unit) final;
	void OnUnitCreated(const sc2::Unit * unit) final;
	void OnUnitDestroyed(const sc2::Unit * unit) final;
	void OnBuildingConstructionComplete(const sc2::Unit * unit) final;
	
	const EconomyManager & EconomyManager() const;
	const BuildingManager & BuildingManager() const;
	const ArmyManagment & ArmyManagment() const;
	const EnemyInfo & EnemyInfo() const;
	const ProductionManager & ProductionManager() const;

	void PrintGlobals();
	void GetDebug();
	size_t GetFinishedBasesCount();
	size_t GetActiveCreepCount();

	//! Gets a currnet number of workers needed for perfect harvesting
	//!< \param bool True -> includes hatches/extractors under construction in calculation
	const size_t GetCurrnetWorkerLimit(bool with_build_under_const = false) const;
	const sc2::Unit * FindNearestBase(const sc2::Unit* unit);


	sc2::GameInfo													game_info_;
	sc2::Point2D													enemy_location_;
	sc2::Point3D													startLocation_;
	sc2::Point3D													staging_location_;	
	std::vector<sc2::Point3D>										expansions_;

	std::vector<const sc2::Unit *>									active_creep_tumors_;			
	std::vector<std::pair<const sc2::Unit*, const sc2::Unit*>>		base_queen;					//! Base & Queen pairs 
	
	sc2::Tag  creep_queen			=	0;				//! Tag of queen for spawning creep tumors
	const sc2::Unit * main_base;						//! Pointer to main base
	long long int frame				=	0;				//! Current gamestep
	int upORdown					=   0;				//! Starting position orientation : 1(down) , 2(up)
	bool pool						=	false;			
	bool worker_rally_set			=	false;
	float enemy_angle;

	size_t active_creep_count;
	size_t num_of_workers_in_prod;
	size_t num_of_workers;
	size_t workers_limit;

	bool save_money = false;
	
	float GetDistanceToMain(sc2::Point3D & point);
	std::chrono::steady_clock::time_point start;
	void PerfMeterStart();
	void PerfMeterEnd(std::string module);
	void DebugTextOnScreen(std::string text,std::string value);

};

