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

	scout_pos_index = 0;
	squad_assigned = false;
	target = nullptr;
	sc2::Point2D top_right = bot.Observation()->GetGameInfo().playable_max;
	sc2::Point2D bottom_left = bot.Observation()->GetGameInfo().playable_min;
	sc2::Point2D top_left(bottom_left.x, top_right.y);
	sc2::Point2D bottom_right(top_right.x, bottom_left.y);
	std::vector<sc2::Point2D> corners = { top_left,top_left,bottom_left,bottom_right };
	
	float distance = std::numeric_limits<float>::max();
	for (auto & corner : corners) {
		float d = sc2::DistanceSquared2D(bot.enemy_location_, corner);
		if (d < distance) {
			distance = d;
			enemy_corner = corner;
		}
	}
	distance = std::numeric_limits<float>::max();
	for (auto & corner : corners) {
		float d = sc2::DistanceSquared2D(enemy_corner, corner);
		if (d < distance && d > 5) {
			distance = d;
			escape_corner = corner;
		}
	}
	mutas_in_corner = false;

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
	//! todo : Add units for other races
	sc2::Units enemy_army = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnits({ sc2::UNIT_TYPEID::TERRAN_MARAUDER , sc2::UNIT_TYPEID::TERRAN_MARAUDER,sc2::UNIT_TYPEID::TERRAN_SIEGETANK,sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED,sc2::UNIT_TYPEID::TERRAN_MEDIVAC,sc2::UNIT_TYPEID::TERRAN_WIDOWMINE,sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED }));
	
	//! todo : better rules when to attack
	/*if ((bot.EnemyInfo().GetEnemyArmySupp() * 1.8 < bot.Observation()->GetFoodArmy() && bot.EnemyInfo().GetEnemyArmySupp() > 20) || bot.Observation()->GetFoodUsed()>190) main_army = attack;
	else main_army = defend;*/
	if (bot.Observation()->GetFoodUsed() > 198 && bot.Observation()->GetMinerals() > 800 && bot.Observation()->GetVespene() > 800) main_army = attack;
	else if(bot.Observation()->GetFoodUsed() < 150) main_army = defend;

	bot.DebugTextOnScreen("SAve minerals:", std::to_string(bot.BuildingManager().save_minerals));
	
	if (main_army==defend) {
		auto enemy_clusters = sc2::search::Cluster(enemy_army, 8);
		auto my_clusters = sc2::search::Cluster(zerglings, 3);
		std::vector <std::pair<sc2::Point3D, std::vector<sc2::Unit>>> enemy_attack_clusters;

		for (auto & e_cluster : enemy_clusters) {
			const sc2::Unit * nearest_base = bot.EconomyManager().FindNearestBase(&e_cluster.second.front());
			if (sc2::Distance2D(nearest_base->pos, e_cluster.first) < 30) {
				enemy_attack_clusters.push_back(e_cluster);
			}
		}
		if (enemy_attack_clusters.size() > 0) we_are_under_attack = true;
		else we_are_under_attack = false;

		for (auto & unit : zerglings) {
			if (unit->orders.size() == 0 && sc2::Distance2D(unit->pos, bot.staging_location_)>10 && !we_are_under_attack) {
				bot.Actions()->UnitCommand(unit, sc2::ABILITY_ID::SMART, bot.staging_location_);
			}
		}

		if (enemy_attack_clusters.size() > 0) {
			if (((bot.EnemyInfo().enemy_army_.size()*1.4 < zerglings.size()) && bot.EnemyInfo().enemy_army_.size()<20) || bot.EnemyInfo().enemy_army_.size()>=20 ) {
				bot.Actions()->UnitCommand(zerglings, sc2::ABILITY_ID::ATTACK, enemy_attack_clusters.front().first);
			}
			else {
				for (auto &my_cluster : my_clusters) {
					if (sc2::Distance2D(my_cluster.first,enemy_attack_clusters.front().first)<15) {
						sc2::Point3D retreat_point;
						retreat_point.x = 3 * my_cluster.first.x - enemy_attack_clusters.front().first.x;
						retreat_point.y = 3 * my_cluster.first.y - enemy_attack_clusters.front().first.y;
						for (auto &unit : my_cluster.second) {
							const sc2::Unit *unit_def = &unit;
							bot.Actions()->UnitCommand(unit_def, sc2::ABILITY_ID::SMART, retreat_point);
						}
						
					}
				}
			}
		}
		



		bool attack_sent = false;
	}

	if (main_army==attack) {
			bot.Actions()->UnitCommand(zerglings, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.back().pos);
			attack_sent = true;
		
	}

	
}

void ArmyManagment::MutaManager()
{
	
	sc2::Units anti_air_army = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnits({ sc2::UNIT_TYPEID::TERRAN_MARINE, sc2::UNIT_TYPEID::TERRAN_WIDOWMINE, sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER, sc2::UNIT_TYPEID::TERRAN_THOR,  sc2::UNIT_TYPEID::TERRAN_LIBERATOR, sc2::UNIT_TYPEID::TERRAN_MISSILETURRET }));
	sc2::Units workers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnits({ sc2::UNIT_TYPEID::TERRAN_SCV,sc2::UNIT_TYPEID::ZERG_DRONE,sc2::UNIT_TYPEID::PROTOSS_PROBE }));
	
	sc2::Units workers_in_range_1;
	sc2::Units workers_in_range_2;
	if (muta_squad_1.size() == 0) {
		
		if (mutas.size() > 0) {
			muta_squad_1.push_back(mutas.front());
			mutas.erase(mutas.begin());
		}
	}

	//New mutas
	if (muta_squad_1.size() > 0) {
		auto enemy_clusters = sc2::search::Cluster(anti_air_army, 10);
		const sc2::Unit * muta_leader = muta_squad_1.front();

		for (auto & muta : mutas) {
			auto closest_worker = FindNearestEnemyUnit(muta, sc2::UNIT_TYPEID::TERRAN_SCV);
			auto anti_air = FindNearstEnemyCluster(muta, enemy_clusters);
			if (sc2::Distance2D(muta->pos, muta_leader->pos) < 6) {
				muta_squad_1.push_back(muta);
				mutas.erase(std::remove_if(mutas.begin(), mutas.end(),
					[&](const sc2::Unit * muta_) { return muta->tag == muta_->tag; }),
					mutas.end());
			}
			else if(sc2::Distance2D(anti_air.first,muta->pos)<16){
				sc2::Point2D around_point= GetAroundPoint(anti_air.first, muta->pos, muta_leader->pos);
				bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::SMART, GetRetreatPoing(anti_air.first, muta->pos));
			}
			else if(closest_worker && sc2::Distance2D(closest_worker->pos,muta->pos)<9)  {
				bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::ATTACK, closest_worker);
			}
			else{ 
				bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::SMART, muta_leader);
			}
		}

		


		auto muta_cluster = sc2::search::Cluster(muta_squad_1, 5).front();
		auto closest_worker = FindNearestEnemyUnit(muta_leader, sc2::UNIT_TYPEID::TERRAN_SCV);
		auto anti_air_2 = FindNearstEnemyCluster(muta_leader, enemy_clusters);
		bot.Debug()->DebugSphereOut(anti_air_2.first, 7, sc2::Colors::Red);
		for (auto &unit : anti_air_2.second) {
			bot.Debug()->DebugSphereOut(unit.pos, 2, sc2::Colors::Purple);
		}
		bot.Debug()->SendDebug();
		
		
		if (sc2::Distance2D(muta_cluster.first, enemy_corner) < 2 && !mutas_in_corner) { mutas_in_corner = true; }
		else if (sc2::Distance2D(muta_cluster.first, enemy_corner) > 40) { mutas_in_corner = false; }

		if (mutas_in_corner) {
			bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::SMART, escape_corner);
		}

		else {
			if (sc2::Distance2D(anti_air_2.first, muta_cluster.first) < 13) {
				if ((anti_air_2.second.size() < 6 && muta_squad_1.size() > 13) || (anti_air_2.second.size() < 3 && muta_squad_1.size() > 9) || (anti_air_2.second.size() < 2 && muta_squad_1.size() > 6)) {
					const sc2::Unit * unit = bot.Observation()->GetUnit(anti_air_2.second.front().tag);
					bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::ATTACK, unit);
				}
				else {
						bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::SMART, GetRetreatPoing(anti_air_2.first, muta_cluster.first));
				}
			}
			else if (closest_worker) {
				if (sc2::Distance2D(closest_worker->pos, muta_cluster.first) < 10) {
					bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::ATTACK, closest_worker);
				}
			}


			if (muta_leader->orders.size() == 0 && muta_squad_1.size() > 4) {
				bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.back().pos);
			}
			else if (muta_leader->orders.size() == 0 && muta_squad_1.size() < 5) {
				bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::SMART, bot.startLocation_);
			}



			for (auto &attack_muta : muta_squad_1) {
				if (sc2::Distance2D(attack_muta->pos, muta_leader->pos) > 4) {
					bot.Actions()->UnitCommand(attack_muta, sc2::ABILITY_ID::SMART, muta_leader);
				}
			}
		}
	}
	

	//


	//sc2::Point2D attack_pos_1;
	//sc2::Point2D attack_pos_2;

	//size_t i = bot.EnemyInfo().enemy_bases_.size();
	//if (i > 1) {
	//	attack_pos_1 = bot.EnemyInfo().enemy_bases_[i - 1].pos;
	//	attack_pos_2 = bot.EnemyInfo().enemy_bases_[i - 2].pos;
	//}
	//else if(i == 1){
	//	attack_pos_1 = bot.EnemyInfo().enemy_bases_[i - 1].pos;
	//	attack_pos_2 = bot.EnemyInfo().enemy_bases_[i - 1].pos;
	//}
	//for (auto& worker : workers) {
	//	if (muta_squad_1.size() > 0) {
	//		if (sc2::Distance2D(muta_squad_1[0]->pos, worker->pos) < 10) {
	//			workers_in_range_1.push_back(worker);
	//		}
	//	}
	//	if (!muta_squad_1_sent && muta_squad_1.size() > 14) {
	//		bot.Actions()->UnitCommand(muta_squad_1, sc2::ABILITY_ID::ATTACK, attack_pos_1);
	//		muta_squad_1_sent = true;
	//	}

	//	if (muta_squad_1.size() > 0) {
	//		/*if (bot.frame % 100 == 0) {
	//			auto muta_clusters = sc2::search::Cluster(muta_squad_1, 4);
	//			for (auto & cluster : muta_clusters) {
	//				for (auto & muta : cluster.second) {
	//					bot.Actions()->UnitCommand(&muta, sc2::ABILITY_ID::SMART, cluster.first);
	//				}
	//			}
	//		}*/
	//		
	//		for (auto &muta : muta_squad_1) {
	//			auto anti_air_1 = FindNearstEnemyCluster(muta, enemy_clusters);
	//			auto closest_worker = FindNearestEnemyUnit(muta, sc2::UNIT_TYPEID::TERRAN_SCV);
	//			auto closest_turret = FindNearestEnemyUnit(muta, sc2::UNIT_TYPEID::TERRAN_MISSILETURRET);
	//			bot.Debug()->DebugSphereOut(anti_air_1.first, 7, sc2::Colors::Red);
	//			if (closest_worker) {
	//				if (sc2::Distance2D(closest_worker->pos, muta->pos) < 10 && target == nullptr) {
	//					target = closest_worker;
	//				}
	//			}
	//			if (closest_turret) {
	//				if (sc2::Distance2D(closest_turret->pos, muta->pos) < 6 && !turret_found) {
	//					target = closest_turret;
	//					turret_found = true;
	//				}
	//			}
	//			if (sc2::Distance2D(muta->pos, anti_air_1.first) < 10 || muta->health < muta->health_max * 0.3) {
	//				sc2::Point3D retreat_point;
	//				retreat_point.x = 2 * muta->pos.x - anti_air_1.first.x;
	//				retreat_point.y = 2 * muta->pos.y - anti_air_1.first.y;
	//				bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::SMART, retreat_point);
	//			}
	//		/*	else if (workers_in_range_1.size() > 0 && muta->orders.size() == 0) {
	//				bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::ATTACK, workers_in_range_1.front());
	//				bot.Debug()->DebugSphereOut(workers_in_range_1.front()->pos, 3, sc2::Colors::White);

	//			}*/
	//			else if (muta_squad_1_sent && muta->orders.size() == 0 && target != nullptr) {
	//				if (target->is_on_screen) {
	//					bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::ATTACK, target);
	//				}
	//				else {
	//					bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::ATTACK, target->pos);
	//				}
	//				bot.Debug()->DebugSphereOut(target->pos, 3, sc2::Colors::White);

	//			}
	//			else if (muta_squad_1_sent && muta->orders.size() == 0 ) {
	//				bot.Actions()->UnitCommand(muta, sc2::ABILITY_ID::ATTACK, attack_pos_1);
	//				sc2::Point3D attack_pos_1_debug(attack_pos_1.x, attack_pos_1.y, 2);
	//				bot.Debug()->DebugSphereOut(attack_pos_1_debug, 3, sc2::Colors::White);

	//			}
	//		}
	//	}
	//}
	//if (target != nullptr) {
	//	auto target_last = bot.Observation()->GetUnit(target->tag);
	//	if (!target_last) {
	//			target = nullptr;
	//			turret_found = false;
	//	}
	//}
	//bot.Debug()->SendDebug();
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


	if (main_army_.size() > 30 && harass_squad_1.empty() && harass_squad_2.empty()) {
		harass_squad_1.assign(main_army_.begin(), main_army_.begin() + 6);
		main_army_.erase(main_army_.begin(), main_army_.begin() + 6);
		harass_squad_2.assign(main_army_.begin(), main_army_.begin() + 6);
		main_army_.erase(main_army_.begin(), main_army_.begin() + 6);
		bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::SMART, harass_squad_1.front()->pos);
		bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::SMART, harass_squad_2.front()->pos);

	}
	if (bot.EnemyInfo().enemy_bases_.size() > 2) {
		
		if (!harass_squad_1.empty()) {
			// if squad units are grouped and squad leader has no orders -> go to the enemy base
			if (UnitsGrouped(harass_squad_1, 4) && harass_squad_1.front()->orders.empty()) {
				bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::SMART, bot.EnemyInfo().enemy_bases_.end()[-2].pos );
			}
			// else if no orders group squad
			else if (harass_squad_1.front()->orders.empty()) {
				GroupUnits(harass_squad_1);
			}
			// attack order
			auto scv = FindNearestEnemyUnit(harass_squad_1.front(), sc2::UNIT_TYPEID::TERRAN_SCV);
			auto turret = FindNearestEnemyUnit(harass_squad_1.front(), sc2::UNIT_TYPEID::TERRAN_MISSILETURRET);

			if (scv && sc2::Distance2D(scv->pos, harass_squad_1.front()->pos) < 15) {
				bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, scv);
			}
			else if (turret && sc2::Distance2D(turret->pos, harass_squad_1.front()->pos) < 15) {
				bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, turret);
			}
			else if (sc2::Distance2D(bot.EnemyInfo().enemy_bases_.end()[-2].pos, harass_squad_1.front()->pos) < 6) {
				bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-2].pos);
			}
		}
		if (!harass_squad_2.empty()) {
			// if squad units are grouped and squad leader has no orders -> go to the enemy base
			if (UnitsGrouped(harass_squad_2, 4) && harass_squad_2.front()->orders.empty()) {
				bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::SMART, bot.EnemyInfo().enemy_bases_.end()[-3].pos);
			}
			// else if no orders group squad
			else if (harass_squad_2.front()->orders.empty()) {
				GroupUnits(harass_squad_2);
			}
			// attack order
			auto scv = FindNearestEnemyUnit(harass_squad_2.front(), sc2::UNIT_TYPEID::TERRAN_SCV);
			auto turret = FindNearestEnemyUnit(harass_squad_2.front(), sc2::UNIT_TYPEID::TERRAN_MISSILETURRET);
			/*auto base = FindNearestEnemyUnit(harass_squad_2.front(), sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND);*/

			if (scv && sc2::Distance2D(scv->pos, harass_squad_2.front()->pos) < 15) {
				bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, scv);
			}
			else if (turret && sc2::Distance2D(turret->pos, harass_squad_2.front()->pos) < 15) {
				bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, turret);
			}
			else if (sc2::Distance2D(bot.EnemyInfo().enemy_bases_.end()[-3].pos, harass_squad_2.front()->pos) < 6) {
				bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-3].pos);
			}


		}
	}
	//int i = 0;
	//if (harass_squad_1.size() == 0 && harass_squad_2.size() == 0) squad_assigned = false;
	//if (army.size() > 20 && !squad_assigned) {
	//	for (auto &unit : army) {
	//		if (i < 7) {
	//			harass_squad_1.push_back(unit);
	//		}
	//		else if (i < 15) {
	//			harass_squad_2.push_back(unit);
	//		}
	//		i++;
	//	}
	//	squad_assigned = true;
	//}

	//if (squad_assigned && main_army==defend) {

	//	//check if units are near eachother

	//	auto nearest_enemy_1 = FindNearestEnemyArmy(harass_squad_1.front());
	//	auto nearest_enemy_2 = FindNearestEnemyArmy(harass_squad_2.front());
	//	if (nearest_enemy_1 != nullptr) {
	//		if (sc2::Distance2D(nearest_enemy_1->pos, harass_squad_1.front()->pos) < 7) {
	//			bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, bot.startLocation_);
	//		}
	//		else if (sc2::Distance2D(nearest_enemy_1->pos, harass_squad_1.front()->pos) > 15) {
	//			if (GroupUnits(harass_squad_1, 6)) {
	//				bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-1].pos);
	//			}
	//		}
	//	}
	//	else { 
	//		if (GroupUnits(harass_squad_1, 6)) {
	//			bot.Actions()->UnitCommand(harass_squad_1, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-1].pos);
	//		}
	//	}

	//	if(nearest_enemy_2 != nullptr){
	//		if (sc2::Distance2D(nearest_enemy_2->pos, harass_squad_2.front()->pos) < 7) {
	//				bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, bot.staging_location_);
	//			}
	//		else if (sc2::Distance2D(nearest_enemy_2->pos, harass_squad_2.front()->pos) > 15) {
	//			if (GroupUnits(harass_squad_2, 6)) {
	//				bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-2].pos);
	//			}
	//		}
	//	}
	//	else {
	//		if (GroupUnits(harass_squad_2, 6)) {
	//			bot.Actions()->UnitCommand(harass_squad_2, sc2::ABILITY_ID::ATTACK, bot.EnemyInfo().enemy_bases_.end()[-2].pos);
	//		}
	//	}
	//	
	//	bot.Debug()->DebugSphereOut(bot.EnemyInfo().enemy_bases_.end()[-1].pos, 10);
	//	bot.Debug()->DebugSphereOut(bot.EnemyInfo().enemy_bases_.end()[-2].pos, 10, sc2::Colors::Red);
	//}

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
			if (ovi_loc.first->orders.size() == 0 && sc2::Distance2D(ovi_loc.first->pos, ovi_loc.second) > 30 ) {
				bot.Actions()->UnitCommand(ovi_loc.first, sc2::ABILITY_ID::SMART, ovi_loc.second);
				i++;
				continue;
			}

			const sc2::Unit *nearest_enemy = FindNearestEnemy(ovi_loc.first);
			if (nearest_enemy == nullptr ||  !IsArmy(nearest_enemy)) continue;
			if (sc2::Distance2D(ovi_loc.first->pos, nearest_enemy->pos) < 11) {
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
	sc2::Units banes = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_BANELING,sc2::UNIT_TYPEID::ZERG_BANELINGCOCOON }));
	sc2::Units all = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_ZERGLING , sc2::UNIT_TYPEID::ZERG_BANELING, sc2::UNIT_TYPEID::ZERG_BANELINGCOCOON }));
	sc2::Units zerglings = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit( sc2::UNIT_TYPEID::ZERG_ZERGLING ));
	const sc2::Unit *candidate=nullptr;
	if (banes.size() < all.size() / 2) {
		for (auto &ling : main_army_) {
			if (ling->unit_type.ToType() != sc2::UNIT_TYPEID::ZERG_ZERGLING) continue;
			auto enemy = FindNearestEnemyArmy(ling);
			if (enemy == nullptr) { candidate = ling; break; }
			if (sc2::Distance2D(FindNearestEnemyArmy(ling)->pos, ling->pos) > 25) {
				candidate = ling;
				break;
			}
		}
		if (candidate) {
			bot.Actions()->UnitCommand(candidate, sc2::ABILITY_ID::TRAIN_BANELING);
		}
	}
}


sc2::Point2D ArmyManagment::GetAroundPoint(sc2::Point2D enemy_pos, sc2::Point2D unit_pos, sc2::Point2D end_pos)
{
	sc2::Point2D around_point_1;
	sc2::Point2D around_point_2;
	const float pi = 3.14;
	float dx = unit_pos.x - enemy_pos.x;
	float dy = unit_pos.y - enemy_pos.y;
	float angle = atan2(dx, dy);
	around_point_1.x = unit_pos.x + (10 * cos(angle + pi / 1.3));
	around_point_1.y = unit_pos.y + (10 * sin(angle + pi / 1.3));
	around_point_2.x = unit_pos.x + (10 * cos(angle - pi / 1.3));
	around_point_2.y = unit_pos.y + (10 * sin(angle - pi / 1.3));
	if (sc2::Distance2D(around_point_1, end_pos) > sc2::Distance2D(around_point_2, end_pos)) return around_point_2;
	else return around_point_1;
}

sc2::Point2D ArmyManagment::GetRetreatPoing(sc2::Point2D enemy_pos, sc2::Point2D unit_pos)
{
	sc2::Point2D retreat_point;
	retreat_point.x = 2 * unit_pos.x - enemy_pos.x;
	retreat_point.y = 2 * unit_pos.y - enemy_pos.y;
	return retreat_point;
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

const sc2::Unit * ArmyManagment::FindNearestEnemyUnit(const sc2::Unit * unit, sc2::UNIT_TYPEID unit_type)
{
	sc2::Units enemy_units = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy,sc2::IsUnit(unit_type));
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


bool ArmyManagment::UnitsGrouped(sc2::Units units, float radius)
{
	for (auto & unit : units) {
		if (sc2::Distance2D(units.front()->pos, unit->pos) > radius)return false;
	}
	return true;
	
}

void ArmyManagment::GroupUnits(sc2::Units units)
{
	for (auto &unit : units) {
		bot.Actions()->UnitCommand(unit, sc2::ABILITY_ID::SMART, units.front()->pos);
	}
}

	

