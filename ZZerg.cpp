#include <iostream>
#include <string>
#include <algorithm>
#include <random>

#include <iterator>
#include "ZZerg.h"



#include <chrono>
#include <math.h>

ZZerg::ZZerg() : bld(*this), eco(*this),army(*this),enemy_info(*this),production(*this)
{
}

ZZerg::~ZZerg() 
{
	
}


void ZZerg::OnGameStart()
{
	// todo: calculate expansions with pathing distance not just distance
	game_info_ = Observation()->GetGameInfo();
	expansions_ = sc2::search::CalculateExpansionLocations(Observation(), Query());
	startLocation_ = Observation()->GetStartLocation();
	staging_location_ = startLocation_;
	enemy_location_ = game_info_.enemy_start_locations.front();
	
	std::sort(expansions_.begin(),expansions_.end(),
		[this](sc2::Point3D& lhs,sc2::Point3D& rhs) { return GetDistanceToMain(lhs) > GetDistanceToMain(rhs); });
	
	//Spawn location up or down?
	if (startLocation_.y - enemy_location_.y < 0) {
		upORdown = 1; //down  todo:better
	}
	else {
		upORdown = 2; // up
	}
	
	for (auto &base : Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_HATCHERY))) {
		base_queen.push_back(std::make_pair(base, nullptr));
		main_base = base;
	}

	for (auto & scout_loc : expansions_) {
		army.ovi_scout_loc_.push_back(std::make_pair(nullptr, scout_loc));
	}
	sc2::Units overlords = Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_OVERLORD));
	army.ovi_scout_loc_.front().first = overlords.front();
	enemy_info.EnemyInfoInit();
	army.init();
	eco.drones_per_ext = 0;
}

void ZZerg::OnStep()
{
	
	PerfMeterStart();
		bld.BuildingManagerStep();
	PerfMeterEnd("Building Manager : ");

	PerfMeterStart();
		production.All();
	PerfMeterEnd("Production Manager : ");
	
	PerfMeterStart();
		eco.EconomyManagerAll();
	PerfMeterEnd("Economy Manager : ");

	PerfMeterStart();
		army.ArmyManagmentAll();
	PerfMeterEnd("Army Manager : ");

	PerfMeterStart();
		enemy_info.EnemyInfoAll();
	PerfMeterEnd("Enemy Info : ");

	PrintGlobals();
	/*GetDebug();*/
	frame++;
}

void ZZerg::OnGameEnd()
{
}

void ZZerg::OnUnitIdle(const sc2::Unit * unit)
{

	switch (unit->unit_type.ToType())
	{
		case sc2::UNIT_TYPEID::ZERG_DRONE: eco.WorkerIdleManager(unit);
		default:
			break;
	}	
}

void ZZerg::OnUnitCreated(const sc2::Unit * unit)
{
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_ZERGLING)
	{
		army.main_army_.push_back(unit);

	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_QUEEN )
	{
		//Checking if there is queenless base 
		//if true, assign created unit to that base
		for (auto & BQpair : base_queen) {
			if (BQpair.first != nullptr && BQpair.second == nullptr) {
				BQpair.second = unit;
				return;
			}
		}
		//If we dont have creep queen, assign 
		if (creep_queen == 0) {
			creep_queen = unit->tag;
			return;
		}
		//rest of created queens become defence squad
		army.defense_queens_.push_back(unit);
	}

	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_CREEPTUMORQUEEN || unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_CREEPTUMOR) 
	{
		//keeping track of active creep tumors
		active_creep_tumors_.push_back(unit);
		//only keep track of last 8 tumors, erase first if more
		if (active_creep_tumors_.size() > 8) {
			active_creep_tumors_.erase(active_creep_tumors_.begin());
		}
		
	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_OVERLORD)
	{	
		for (auto & ovi_loc : army.ovi_scout_loc_) {
			if (ovi_loc.first == nullptr) {
				ovi_loc.first = unit;
				return;
			}
		}
	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_MUTALISK)
	{
		if (army.muta_squad_1.size() == 0) {
			army.muta_squad_1.push_back(unit);
		}
		else {
			army.mutas.push_back(unit);
		}
	}
}

void ZZerg::OnUnitDestroyed(const sc2::Unit * unit)
{
	enemy_info.EnemyUnitDestroyed(*unit);

	army.main_army_.erase(std::remove_if(army.main_army_.begin(), army.main_army_.end(),
		[&](const sc2::Unit * army_unit) { return unit->tag == army_unit->tag; }),
		army.main_army_.end());

	army.harass_squad_1.erase(std::remove_if(army.harass_squad_1.begin(), army.harass_squad_1.end(),
		[&](const sc2::Unit * harass_unit) { return unit->tag == harass_unit->tag; }),
		   army.harass_squad_1.end());
	army.harass_squad_2.erase(std::remove_if(army.harass_squad_2.begin(), army.harass_squad_2.end(),
		[&](const sc2::Unit * harass_unit) { return unit->tag == harass_unit->tag; }),
		army.harass_squad_2.end());

	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_MUTALISK)
	{
		army.muta_squad_1.erase(std::remove_if(army.muta_squad_1.begin(), army.muta_squad_1.end(),
			[&](const sc2::Unit * harass_unit) { return unit->tag == harass_unit->tag; }),
			army.muta_squad_1.end());
		army.mutas.erase(std::remove_if(army.mutas.begin(), army.mutas.end(),
			[&](const sc2::Unit * muta) { return unit->tag == muta->tag; }),
			army.mutas.end());
	}

	


	//If creep queen has died, reset variable
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_QUEEN && unit->tag == creep_queen)
	{
		creep_queen = 0;
	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_QUEEN )
	{
		for (auto & BQpair : base_queen) {
			if (BQpair.second == nullptr) continue;
			if (BQpair.second->tag == unit->tag) {
				BQpair.second = nullptr;
			}
		}
		int i = 0;
		for (auto & defense_queen : army.defense_queens_) {
			if (defense_queen->tag == unit->tag) {
				army.defense_queens_.erase(army.defense_queens_.begin() + i);
			}
			i++;
		}
	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_HATCHERY )
	{
		int i = 0;
		for (auto & BQpair : base_queen) {
			if (BQpair.first == nullptr) continue;
			if (BQpair.first->tag == unit->tag) {
				base_queen.erase(base_queen.begin()+i);
			}
			i++;
		}
	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL)
	{
		pool = false;
	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED || unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_CREEPTUMORQUEEN)
	{
		int i = 0;
		for (auto & active_creep : active_creep_tumors_) {
			if (active_creep->tag == unit->tag) {
				active_creep_tumors_.erase(active_creep_tumors_.begin() + i);
			}
			i++;
		}
	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_OVERLORD)
	{
		int i = 0;
		for (auto & ovi_loc : army.ovi_scout_loc_) {
			if (ovi_loc.first != nullptr) {
				if (unit->tag == ovi_loc.first->tag) {
					ovi_loc.first = nullptr;
				}
			}
			i++;
		}
	}
	

}

void ZZerg::OnBuildingConstructionComplete(const sc2::Unit * unit)
{

	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_HATCHERY )
	{
		base_queen.push_back(std::make_pair(unit, nullptr));
		
	}
	if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL)
	{
		pool = true;
	}
}


const EconomyManager & ZZerg::EconomyManager() const
{
	return eco;
}

const BuildingManager & ZZerg::BuildingManager() const
{
	return bld;
}

const ArmyManagment & ZZerg::ArmyManagment() const
{
	return army;
}

const EnemyInfo & ZZerg::EnemyInfo() const
{
	return enemy_info;
}

const ProductionManager & ZZerg::ProductionManager() const
{
	return production;
}


void ZZerg::PrintGlobals()
{

	if (frame % 30 == 0) {
		active_creep_count = Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED)).size();
		std::cout << "--------------------" << std::endl;
		std::cout << "frame ->" << frame << std::endl;
		/*std::cout << "Up or Down ->" << upORdown << std::endl;
		std::cout << "Finished bases count ->" << GetFinishedBasesCount() << std::endl;
		std::cout << "Active creep count ->" << GetActiveCreepCount() << std::endl;
		std::cout << "Creep queen ->" << creep_queen << std::endl;*/
		/*for (auto &BQpair : base_queen) {
			std::cout << "base/queen ->" << BQpair.first << "/" << BQpair.second << std::endl;
		}
		std::cout << "--------------------" << std::endl;*/
	/*	for (auto &defence_queen : army.defense_queens_) {
			std::cout << "defender queen ->" << std::to_string(defence_queen->tag) << std::endl;
		}
		std::cout << "--------------------" << std::endl;
		for (auto &creep_tumor : active_creep_tumors_) {
			std::cout << "active creep ->" << std::to_string(creep_tumor->tag) << std::endl;
		}
		std::cout << "--------------------" << std::endl;
		for (auto &ovi_loc : army.ovi_scout_loc_) {
			std::cout << "ovi/loc ->" << ovi_loc.first << "/ " << (int)ovi_loc.second.x << "-" << (int)ovi_loc.second.y << std::endl;
		}*/
		
		std::cout << "--------------------" << std::endl;
		for (auto & enemy : enemy_info.enemy_army_) {
			std::cout << "Unit army-> " << enemy.tag << std::endl;
		}
		
		std::cout << "--------------------" << std::endl;
		for (auto & tech : enemy_info.enemy_buildings) {
			std::cout << "Tech-> " << tech.tag << std::endl;
		}
		std::cout << "--------------------" << std::endl;
	}
}

void ZZerg::GetDebug()
{
	sc2::Units all_units_ = Observation()->GetUnits();
	sc2::UnitTypes all_unit_data = Observation()->GetUnitTypeData();
	for (auto & unit : all_units_) {
		Debug()->DebugTextOut(unit->unit_type.to_string(),unit->pos);
		auto unit2 = unit->pos;
		unit2.y += 1;
		Debug()->DebugTextOut(std::to_string(unit->tag), unit2);
	}
	Debug()->SendDebug();
}


size_t ZZerg::GetFinishedBasesCount()
{
	size_t finished_bases_count = 0;
	for (auto & base : Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_HATCHERY,  sc2::UNIT_TYPEID::ZERG_LAIR,  sc2::UNIT_TYPEID::ZERG_HIVE }))) {
		if (base->build_progress == 1) finished_bases_count++;
	}
	return finished_bases_count;
}

size_t ZZerg::GetActiveCreepCount()
{
	active_creep_count = 0;
	sc2::Units all_creep = Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED));
	for (auto &tumor : all_creep) {
		if (Query()->GetAbilitiesForUnit(tumor, true).abilities.size() > 0) {
			active_creep_count++;
		}
	}
	return active_creep_count;
}


const size_t ZZerg::GetCurrnetWorkerLimit(bool with_build_under_const) const
{
	sc2::Units bases = Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_HATCHERY, sc2::UNIT_TYPEID::ZERG_LAIR, sc2::UNIT_TYPEID::ZERG_HIVE }));
	sc2::Units extractors = Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EXTRACTOR));
	size_t worker_limit = 0;
	size_t build_under_const = 0;

	for (auto &base : bases) {
		if (base->build_progress != 1) {
			build_under_const = build_under_const + 16;
			continue;
		}
		worker_limit = worker_limit + base->ideal_harvesters;
	}
	for (auto &extr : extractors) {
		if (extr->build_progress != 1) {
			build_under_const = build_under_const + 3;
			continue;
		}
		worker_limit = worker_limit + extr->ideal_harvesters;
	}
	if (!with_build_under_const) return worker_limit;
	return worker_limit + build_under_const;

}

const sc2::Unit * ZZerg::FindNearestBase(const sc2::Unit* unit)
{
	sc2::Units bases = Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_HATCHERY));
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &base : bases) {
		float d = sc2::DistanceSquared2D(base->pos, unit->pos);
		if (d < distance) {
			distance = d;
			target_node = base;
		}
	}
	return target_node;
}

float ZZerg::GetDistanceToMain(sc2::Point3D &point)
{
	return sc2::Distance2D(point, startLocation_);
}

void ZZerg::PerfMeterStart()
{
	start = std::chrono::high_resolution_clock::now();

}

void ZZerg::PerfMeterEnd(std::string module)
{
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = stop - start;
	if (frame % 30 == 0)std::cout << module << elapsed.count() << " s\n";
}

void ZZerg::DebugTextOnScreen(std::string text, std::string value)
{
	Debug()->DebugTextOut(text + ": " + value);
	Debug()->SendDebug();
}





