#pragma once
class ZZerg;


class ArmyManagment
{
	
	ZZerg &bot;
public:
	ArmyManagment(ZZerg &bot);
	~ArmyManagment();
	
	void init();
	void ArmyManagmentAll();
	
	void MainArmyManager();
	void MutaManager();
	void QueenDefenceManager();
	void HarassManager();
	void ScoutManager();
	void MorphUnits(); // move to production
	void ClusterTest();
	
	enum army_state { defend, attack, harass };
	army_state main_army;
	float GetClusterValue(sc2::Point3D cluster_pos, std::vector<sc2::Unit> &units);

	//! return nearest enemy unit to *unit
	const sc2::Unit * FindNearestEnemy(const sc2::Unit* unit);
	//! return nearest enemy army unit to *unit
	const sc2::Unit * FindNearestEnemyArmy(const sc2::Unit* unit);
	//! return queen under 50% hp in radius
	const sc2::Unit * FindLowHpQueenInRad(const sc2::Unit* unit,float radius);
	//! return nearest enemy base from bot starting location
	const sc2::Unit * FindNearestEnemyBase();

	std::pair<sc2::Point3D, std::vector<sc2::Unit>> FindNearstEnemyCluster(const sc2::Unit *unit, std::vector<std::pair<sc2::Point3D, std::vector<sc2::Unit>>> enemy_clusters);


	bool IsStructure(const sc2::Unit *unit)const;
	bool IsArmy(const sc2::Unit *unit)const;
	bool UnitsGrouped(std::vector<const sc2::Unit *> &units, float radius);
	bool GroupUnits(std::vector<const sc2::Unit *> &units, float radius);
	std::vector <const sc2::Unit*>								defense_queens_;
	std::vector<std::pair<const sc2::Unit*, sc2::Point3D>>		ovi_scout_loc_;
	std::vector<sc2::Point2D>									inital_scout_pos_;
	std::vector<sc2::Point2D>									lingbane_harass_pos_;
	sc2::Units													harass_squad_1;
	sc2::Units													harass_squad_2;
	sc2::Units													muta_squad_1;
	sc2::Units													muta_squad_2;

	const sc2::Unit *scout;
	bool attack_sent;
	int scout_pos_index;
	bool squad_assigned;
	bool we_are_under_attack;
	bool muta_squad_1_sent;
	bool muta_squad_2_sent;
	
};

