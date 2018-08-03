#include <iostream>
#include <math.h>

#include "ZZerg.h"
#include "sc2lib/sc2_lib.h"
#include "Utility.h"



ArmyManagment::ArmyManagment(ZZerg &bot) : bot(bot)
{
}


ArmyManagment::~ArmyManagment()
{
}

void ArmyManagment::init()
{

	sc2::Units workers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_DRONE));

	//get random worker and assign it as scout
	scout = sc2::GetRandomEntry(workers);
	
	//inital scout target positions
	const float pi = 3.14;
	for (float z = 0, x = 0; z < 4; z++, x = x + pi / 2) {
		inital_scout_pos_.push_back(sc2::Point2D(bot.enemy_location_.x + (9 * cos(x)), bot.enemy_location_.y + (9 * sin(x))));
	}
	//send scout to enemy base
	bot.Actions()->UnitCommand(scout, sc2::ABILITY_ID::SMART, inital_scout_pos_[1]);
	scout_pos_index=0;
	squad_assigned = false;

}

void ArmyManagment::ArmyManagmentAll()
{
	MainArmyManager();
	MutaManager();
	QueenDefenceManager();
	HarassManager();
	MorphUnits();
	ScoutManager();
	//ClusterTest();
}

void ArmyManagment::MainArmyManager()
{
	sc2::Units zerglings = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_ZERGLING , sc2::UNIT_TYPEID::ZERG_BANELING }));
	sc2::Units bane_nest = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_BANELINGNEST));
	sc2::Units banes = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_BANELING));
	sc2::Units enemy_army = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnits({ sc2::UNIT_TYPEID::TERRAN_MARAUDER , sc2::UNIT_TYPEID::TERRAN_MARAUDER,sc2::UNIT_TYPEID::TERRAN_SIEGETANK,sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED,sc2::UNIT_TYPEID::TERRAN_MEDIVAC,sc2::UNIT_TYPEID::TERRAN_WIDOWMINE,sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED }));
	sc2::Units enemy_bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnits({ sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER,sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER }));
	
	if ((bot.EnemyInfo().GetEnemyArmySupp() * 1.8 < bot.Observation()->GetFoodArmy() && bot.EnemyInfo().GetEnemyArmySupp() > 20) || bot.Observation()->GetFoodUsed()>190) main_army = attack;
	else main_army = defend;
	bot.DebugTextOnScreen("Main Army state:", std::to_string(main_army));
	
	
	//
	if (main_army==defend) {
		//If 
		for (auto & unit : zerglings) {
			if (unit->orders.size() == 0 && sc2::Distance2D(unit->pos,bot.staging_location_)>20) {
				bot.Actions()->UnitCommand(unit, sc2::ABILITY_ID::SMART, bot.staging_location_);
			}
		}

		//Defend when not attacking
		auto enemy_clusters = sc2::search::Cluster(enemy_army, 10);
		std::vector <std::pair<sc2::Point3D, std::vector<sc2::Unit>>> enemy_attack_clusters;
		for (auto & e_cluster : enemy_clusters) {
			const sc2::Unit * nearest_base = bot.EconomyManager().FindNearestBase(&e_cluster.second.front());
			if (sc2::Distance2D(nearest_base->pos, e_cluster.first) < 20) {
				enemy_attack_clusters.push_back(e_cluster);
			}
		}
		if (enemy_attack_clusters.size() > 0) we_are_under_attack = true;
		else we_are_under_attack = false;

		if (enemy_attack_clusters.size() == 1) {
			bot.Actions()->UnitCommand(zerglings, sc2::ABILITY_ID::ATTACK, enemy_attack_clusters.front().first);
		}
		if (enemy_attack_clusters.size() == 2) {
			sc2::Units def_squad_1(zerglings.begin(), zerglings.begin() + enemy_attack_clusters[1].second.size() + 10);
			bot.Actions()->UnitCommand(def_squad_1, sc2::ABILITY_ID::ATTACK, enemy_attack_clusters[1].first);
		}



		bool attack_sent = false;
	}

	if (main_army==attack) {
		if (bot.EnemyInfo().enemy_bases_.size() > 2 ) {
			int half = zerglings.size() / 3;
			sc2::Units att_squad_1(zerglings.begin(), zerglings.begin() + half);
			sc2::Units att_squad_2(zerglings.begin()+ half, zerglings.end());
			bot.Actions()->UnitCommand(att_squad_1, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-2].pos);
			bot.Actions()->UnitCommand(att_squad_2, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-1].pos);
			attack_sent = true;
		}
		if (bot.EnemyInfo().enemy_bases_.size() <= 2 ) {
			bot.Actions()->UnitCommand(zerglings, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.back().pos);
			attack_sent = true;
		}
	}

	
}

void ArmyManagment::MutaManager()
{
	
	sc2::Units anti_air_army = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnits({ sc2::UNIT_TYPEID::TERRAN_MARINE, sc2::UNIT_TYPEID::TERRAN_WIDOWMINE, sc2::UNIT_TYPEID::TERRAN_VIKINGASSAULT, sc2::UNIT_TYPEID::TERRAN_THOR, sc2::UNIT_TYPEID::TERRAN_MISSILETURRET }));
	sc2::Units workers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnits({ sc2::UNIT_TYPEID::TERRAN_SCV,sc2::UNIT_TYPEID::ZERG_DRONE,sc2::UNIT_TYPEID::PROTOSS_PROBE }));
	auto enemy_clusters = sc2::search::Cluster(anti_air_army, 3);
	sc2::Units workers_in_range_1;
	sc2::Units workers_in_range_2;

	sc2::Point2D attack_pos_1;
	sc2::Point2D attack_pos_2;

	size_t i = bot.EnemyInfo().enemy_bases_.size();
	if (i > 1) {
		attack_pos_1 = bot.EnemyInfo().enemy_bases_[i - 1].pos;
		attack_pos_2 = bot.EnemyInfo().enemy_bases_[i - 2].pos;
	}
	else if(i == 1){
		attack_pos_1 = bot.EnemyInfo().enemy_bases_[i - 1].pos;
		attack_pos_2 = bot.EnemyInfo().enemy_bases_[i - 1].pos;
	}
	for (auto& worker : workers) {
		if (muta_squad_1.size() > 0) {
			if (sc2::Distance2D(muta_squad_1[0]->pos, worker->pos) < 10) {
				workers_in_range_1.push_back(worker);
			}
		}
		if (muta_squad_2.size() > 0) {
			if (sc2::Distance2D(muta_squad_2[0]->pos, worker->pos) < 10) {
				workers_in_range_2.push_back(worker);
			}
		}
	}
	if (!muta_squad_1_sent && muta_squad_1.size()>9) {
		bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::ATTACK, attack_pos_1);
		muta_squad_1_sent = true;
	}
	if (!muta_squad_2_sent && muta_squad_2.size()>9) {
		bot.Actions()->UnitCommand(muta_squad_2, sc2::ABILITY_ID::ATTACK, attack_pos_2);
		muta_squad_2_sent = true;
	}
	if (muta_squad_1.size() > 0) {
		auto anti_air_1 = FindNearstEnemyCluster(muta_squad_1[0], enemy_clusters);
		bot.Debug()->DebugSphereOut(anti_air_1.first,8,sc2::Colors::Red);
		if (sc2::Distance2D(anti_air_1.first, muta_squad_1[0]->pos) < 10) {
			if (anti_air_1.second.size() == 1) {
				bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::ATTACK, anti_air_1.first);
				bot.Debug()->DebugSphereOut(anti_air_1.first, 3, sc2::Colors::White);
			}
			else {
				sc2::Point3D retreat_point;
				retreat_point.x = 2 * muta_squad_1[0]->pos.x - anti_air_1.first.x;
				retreat_point.y = 2 * muta_squad_1[0]->pos.y - anti_air_1.first.y;
				bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::SMART,  retreat_point);
				bot.Debug()->DebugSphereOut(retreat_point, 3, sc2::Colors::White);
			}
			
		}
		else if (workers_in_range_1.size() > 0) {
			bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::ATTACK, workers_in_range_1.front());
			bot.Debug()->DebugSphereOut(workers_in_range_1.front()->pos, 3, sc2::Colors::White);

		}
		else if (muta_squad_1_sent) {
			bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::ATTACK, attack_pos_1);
			sc2::Point3D attack_pos_1_debug(attack_pos_1.x, attack_pos_1.y, 2);
			bot.Debug()->DebugSphereOut(attack_pos_1_debug, 3, sc2::Colors::White);
		}

	}
	if (muta_squad_2.size() > 0) {
		auto anti_air_2 = FindNearstEnemyCluster(muta_squad_2[0], enemy_clusters);
		if (sc2::Distance2D(anti_air_2.first, muta_squad_2[0]->pos) < 10) {
			if (anti_air_2.second.size() == 1) {
				bot.Actions()->UnitCommand(muta_squad_2, sc2::ABILITY_ID::ATTACK, anti_air_2.first);
			}
			
			else {
				sc2::Point2D retreat_point2;
				retreat_point2.x = 2 * muta_squad_2[0]->pos.x - anti_air_2.first.x;
				retreat_point2.y = 2 * muta_squad_2[0]->pos.y - anti_air_2.first.y;
				bot.Actions()->UnitCommand(muta_squad_2, sc2::ABILITY_ID::SMART, retreat_point2);
			}
		}
		else if (workers_in_range_2.size() > 0) {
			bot.Actions()->UnitCommand(muta_squad_2, sc2::ABILITY_ID::ATTACK, workers_in_range_2.front());
		}
		else if(muta_squad_2_sent) {
			bot.Actions()->UnitCommand(muta_squad_2, sc2::ABILITY_ID::ATTACK,attack_pos_2);
		}
	}
	
	
	
	

	for (auto &muta : muta_squad_1) {
		if (sc2::Distance2D(muta->pos, muta_squad_1[0]->pos) > 4) {
			bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::SMART, muta_squad_1[0]->pos);
		}

	}

	for (auto &muta : muta_squad_2) {
		if (sc2::Distance2D(muta->pos, muta_squad_2[0]->pos) > 4) {
			bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::SMART, muta_squad_2[0]->pos);
		}

	}
	
	bot.Debug()->SendDebug();
}

void ArmyManagment::QueenDefenceManager()
{
	//Queen defence squad
	for (auto &defender : defense_queens_) {

		if (defender->energy > 49) {
			const sc2::Unit * queen_to_heal = FindLowHpQueenInRad(defender, 8);
			if (queen_to_heal != nullptr) {
				if (queen_to_heal->health < queen_to_heal->health_max * 0.6) {
					bot.Actions()->UnitCommand(defender, sc2::ABILITY_ID::EFFECT_TRANSFUSION, queen_to_heal);
				
					return;
				}
			}
		}
		
			if ((defender->orders.size() ==0) || (defender->orders.size() > 0 && defender->orders.front().ability_id.ToType() != sc2::ABILITY_ID::ATTACK)) {
				sc2::Units enemy_units = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy);
				for (auto & enemy_unit : enemy_units) {
					if (IsArmy(enemy_unit) && sc2::Distance2D(bot.FindNearestBase(enemy_unit)->pos, enemy_unit->pos) < 15) {
						bot.Actions()->UnitCommand(defender, sc2::ABILITY_ID::ATTACK, enemy_unit);
						
						break;
					}
				}
			}
		
		//if queen to far away return to staging pos
		const sc2::Unit * closest_enemy = FindNearestEnemy(defender);
		if (closest_enemy) {
			if (sc2::Distance2D(defender->pos, bot.FindNearestBase(defender)->pos) > 50 && sc2::Distance2D(defender->pos, closest_enemy->pos) > 3) {
				bot.Actions()->UnitCommand(defender, sc2::ABILITY_ID::SMART, bot.staging_location_);
				
			}

			if (sc2::Distance2D(closest_enemy->pos, defender->pos) < 8) {
				bot.Actions()->UnitCommand(defender, sc2::ABILITY_ID::ATTACK, closest_enemy->pos);
			
			}
		}
		if (sc2::Distance2D(defender->pos, bot.staging_location_) > 6 && defender->orders.size() == 0) {
			bot.Actions()->UnitCommand(defender, sc2::ABILITY_ID::SMART, bot.staging_location_);
			/*std::cout << "Go to start location" << std::endl;*/
		}
	
	}
}

void ArmyManagment::HarassManager()
{
	sc2::Units army = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_ZERGLING }));
	int i = 0;
	if (harass_squad_1.size() == 0 && harass_squad_2.size() == 0) squad_assigned = false;
	if (army.size() > 20 && !squad_assigned) {
		for (auto &unit : army) {
			if (i < 7) {
				harass_squad_1.push_back(unit);
			}
			else if (i < 15) {
				harass_squad_2.push_back(unit);
			}
			i++;
		}
		squad_assigned = true;
	}

	if (squad_assigned && main_army==defend) {

		//check if units are near eachother

		auto nearest_enemy_1 = FindNearestEnemyArmy(harass_squad_1.front());
		auto nearest_enemy_2 = FindNearestEnemyArmy(harass_squad_2.front());
		if (nearest_enemy_1 != nullptr) {
			if (sc2::Distance2D(nearest_enemy_1->pos, harass_squad_1.front()->pos) < 7) {
				bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, bot.startLocation_);
			}
			else if (sc2::Distance2D(nearest_enemy_1->pos, harass_squad_1.front()->pos) > 15) {
				if (GroupUnits(harass_squad_1, 6)) {
					bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-1].pos);
				}
			}
		}
		else { 
			if (GroupUnits(harass_squad_1, 6)) {
				bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-1].pos);
			}
		}

		if(nearest_enemy_2 != nullptr){
			if (sc2::Distance2D(nearest_enemy_2->pos, harass_squad_2.front()->pos) < 7) {
					bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, bot.staging_location_);
				}
			else if (sc2::Distance2D(nearest_enemy_2->pos, harass_squad_2.front()->pos) > 15) {
				if (GroupUnits(harass_squad_2, 6)) {
					bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-2].pos);
				}
			}
		}
		else {
			if (GroupUnits(harass_squad_2, 6)) {
				bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-2].pos);
			}
		}
		
		bot.Debug()->DebugSphereOut(bot.EnemyInfo().enemy_bases_.end()[-1].pos, 10);
		bot.Debug()->DebugSphereOut(bot.EnemyInfo().enemy_bases_.end()[-2].pos, 10, sc2::Colors::Red);
	}

}

void ArmyManagment::ScoutManager()
{
	if (scout->orders.size() == 0 && scout->is_alive) {
		bot.Actions()->UnitCommand(scout, sc2::ABILITY_ID::SMART, inital_scout_pos_[scout_pos_index++]);
	}
	if (scout_pos_index == 4) scout_pos_index = 0;
	
	

	int i = 0;
	for (auto & ovi_loc : ovi_scout_loc_) {
		if (i == 0 || i == 1) { 
			i++;
			continue; 
		}
		if (ovi_loc.first != nullptr) {
			if (ovi_loc.first->orders.size() == 0 && sc2::Distance2D(ovi_loc.first->pos, ovi_loc.second) > 50 ) {
				bot.Actions()->UnitCommand(ovi_loc.first, sc2::ABILITY_ID::SMART, ovi_loc.second);
				i++;
				continue;
			}

			const sc2::Unit *nearest_enemy = FindNearestEnemy(ovi_loc.first);
			if (nearest_enemy == nullptr ||  !IsArmy(nearest_enemy)) continue;
			if (sc2::Distance2D(ovi_loc.first->pos, nearest_enemy->pos) < 15) {
				sc2::Point2D retreat_point;
				retreat_point.x = 2 * ovi_loc.first->pos.x - nearest_enemy->pos.x;
				retreat_point.y = 2 * ovi_loc.first->pos.y - nearest_enemy->pos.y;
				bot.Actions()->UnitCommand(ovi_loc.first, sc2::ABILITY_ID::SMART, retreat_point);
			}
		}
		i++;
	}
	
}

void ArmyManagment::MorphUnits()
{
	sc2::Units banes = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_BANELING));
	sc2::Units zerglings = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_ZERGLING , sc2::UNIT_TYPEID::ZERG_BANELING }));
	if (banes.size() < zerglings.size() / 2) {
		bot.Actions()->UnitCommand(sc2::GetRandomEntry(zerglings), sc2::ABILITY_ID::TRAIN_BANELING);
	}
}

void ArmyManagment::ClusterTest()
{
	sc2::Units enemy_units = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy);
	auto Cluster = sc2::search::Cluster(enemy_units, 10);
	auto unit_data = bot.Observation()->GetUnitTypeData();
	for (auto & clus : Cluster) {
		float value = GetClusterValue(clus.first, clus.second);
		bot.Debug()->DebugTextOut(std::to_string(value),clus.first);
		/*for (auto & cl : clus.second) {
			bot.Debug()->DebugSphereOut(cl.pos, 1);
			bot.Debug()->DebugLineOut(cl.pos, clus.first,sc2::Colors::Red);
		}*/
		bot.Debug()->SendDebug();
	}
}

float ArmyManagment::GetClusterValue(sc2::Point3D cluster_pos, std::vector<sc2::Unit> &units)
{
	float total = 0;
	float food_req = 0;
	for (const sc2::Unit cluster_unit : units) {
		if (!IsArmy(&cluster_unit)) continue;
		auto unit_data = bot.Observation()->GetUnitTypeData();
		food_req += unit_data[cluster_unit.unit_type].food_required;
	}
	total = food_req;
	return total;
}

const sc2::Unit * ArmyManagment::FindNearestEnemy(const sc2::Unit * unit)
{
	sc2::Units enemy_units = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy);
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &enemy : enemy_units) {
		float d = sc2::DistanceSquared2D(enemy->pos, unit->pos);
		if (d < distance) {
			distance = d;
			target_node = enemy;
		}
	}
	return target_node;
	
}

const sc2::Unit * ArmyManagment::FindNearestEnemyArmy(const sc2::Unit * unit)
{
	sc2::Units enemy_units = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy,Utility::IsCombatUnit());
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &enemy : enemy_units) {
		float d = sc2::DistanceSquared2D(enemy->pos, unit->pos);
		if (d < distance) {
			distance = d;
			target_node = enemy;
		}
	}
	return target_node;

}

const sc2::Unit * ArmyManagment::FindLowHpQueenInRad(const sc2::Unit * unit,float radius)
{
	float health_min = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &defender : defense_queens_) {
		if (defender->health < health_min && sc2::Distance2D(defender->pos,unit->pos)<radius) {
			health_min = defender->health;
			target_node = defender;
		}
	}
	return target_node;
}

const sc2::Unit * ArmyManagment::FindNearestEnemyBase()
{
	sc2::Units enemy_bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER,sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER }));
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &enemy : bot.EnemyInfo().enemy_buildings) {
		if (enemy.unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER || enemy.unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS || enemy.unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND) {
			float d = sc2::DistanceSquared2D(enemy.pos, bot.startLocation_);
			if (d < distance) {
				distance = d;
				target_node = &enemy;
			}
		}
	}
	return target_node;
}

std::pair<sc2::Point3D, std::vector<sc2::Unit>> ArmyManagment::FindNearstEnemyCluster(const sc2::Unit * unit, std::vector<std::pair<sc2::Point3D, std::vector<sc2::Unit>>> enemy_clusters)
{
	float distance = std::numeric_limits<float>::max();
	std::pair<sc2::Point3D, std::vector<sc2::Unit>> target_node ;

	for (const auto &enemy_cluster : enemy_clusters) {
		float d = sc2::DistanceSquared2D(enemy_cluster.first, unit->pos);
		if (d < distance) {
			distance = d;
			target_node = enemy_cluster;
		}
	}
	return target_node;
}

bool ArmyManagment::IsStructure(const sc2::Unit * unit) const
{
	sc2::UnitTypeData Unit_data = bot.Observation()->GetUnitTypeData().at(unit->unit_type);
	for (auto & attribute : Unit_data.attributes) {
		if (attribute == sc2::Attribute::Structure) {
			return true;
		}
	}
	return false;
}

bool ArmyManagment::IsArmy(const sc2::Unit *unit) const{
	auto attributes = bot.Observation()->GetUnitTypeData().at(unit->unit_type).attributes;
	for (const auto& attribute : attributes) {
			if (attribute == sc2::Attribute::Structure) {
				return false;
			}
		}
		switch (unit->unit_type.ToType()) {
		case sc2::UNIT_TYPEID::ZERG_OVERLORD: return false;
		case sc2::UNIT_TYPEID::PROTOSS_PROBE: return false;
		case sc2::UNIT_TYPEID::ZERG_DRONE: return false;
		case sc2::UNIT_TYPEID::TERRAN_SCV: return false;
		case sc2::UNIT_TYPEID::ZERG_LARVA: return false;
		case sc2::UNIT_TYPEID::ZERG_EGG: return false;
		case sc2::UNIT_TYPEID::TERRAN_MULE: return false;
		case sc2::UNIT_TYPEID::TERRAN_NUKE: return false;
		default: return true;
	}
}

bool ArmyManagment::UnitsGrouped(std::vector<const sc2::Unit*>& units, float radius)
{
	for (auto & unit : units) {
		if (sc2::Distance2D(units.front()->pos, unit->pos) > radius)return false;
	}
	return true;
	
}

bool ArmyManagment::GroupUnits(std::vector<const sc2::Unit*>& units, float radius)
{
	if (!UnitsGrouped(units, radius+5)) {
		bot.Actions()->UnitCommand(units.front(), sc2::ABILITY_ID::HOLDPOSITION);
		for (int i = 1; i < units.size(); i++) {
			if(sc2::Distance2D(units.front()->pos,units[i]->pos) <= radius + 5){
				bot.Actions()->UnitCommand(units[i], sc2::ABILITY_ID::HOLDPOSITION);
			}
			else {
				bot.Actions()->UnitCommand(units[i], sc2::ABILITY_ID::SMART, units.front()->pos);
			}
		}
		return false;
	}
	if (UnitsGrouped(units, radius)) {
		return true;
	}
	return false;
}

	

