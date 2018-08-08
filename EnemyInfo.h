#pragma once


class ZZerg;

class EnemyInfo
{
	ZZerg &bot;
public:
	EnemyInfo(ZZerg &bot);
	~EnemyInfo();

	void EnemyInfoInit();
	void EnemyInfoAll();
	void UpdateEnemyInfo();
	void EnemyUnitDestroyed(const sc2::Unit &unit);



	//! Info about enemy bases
	//! Enemy base 
	std::vector<sc2::Unit>								enemy_bases_;
	std::vector<sc2::Unit>								enemy_army_;
	std::vector<sc2::Unit>								enemy_buildings;
	std::map <float, int>								enemy_unit_count;
	

	bool UnitAlreadySeen(const sc2::Unit *unit, sc2::Units &units);
	float StaticDefCount(sc2::Unit& unit);

	float GetEnemySupply();
	float GetEnemyArmySupp() const;
	float GetEnemyAirCount() const;
};

