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

	
	enum army_state { defend, attack, harass };
	army_state main_army;

	sc2::Point2D GetAroundPoint(sc2::Point2D enemy_pos, sc2::Point2D unit_pos, sc2::Point2D end_pos);
	sc2::Point2D GetRetreatPoing(sc2::Point2D enemy_pos, sc2::Point2D unit_pos);



	//! return nearest enemy unit to *unit
	const sc2::Unit * FindNearestEnemy(const sc2::Unit* unit);
	//! return nearest enemy army unit to *unit
	const sc2::Unit * FindNearestEnemyArmy(const sc2::Unit* unit);
	//! return queen under 50% hp in radius
	const sc2::Unit * FindLowHpQueenInRad(const sc2::Unit* unit,float radius);
	//! return nearest enemy base from bot starting location
	const sc2::Unit * FindNearestEnemyBase();

	const sc2::Unit * FindNearestEnemyUnit(const sc2::Unit* unit,sc2::UNIT_TYPEID unit_type);

	std::pair<sc2::Point3D, std::vector<sc2::Unit>> FindNearstEnemyCluster(const sc2::Unit *unit, std::vector<std::pair<sc2::Point3D, std::vector<sc2::Unit>>> enemy_clusters);
	
	bool IsStructure(const sc2::Unit *unit)const;
	bool IsArmy(const sc2::Unit *unit)const;
	bool UnitsGrouped(sc2::Units units, float radius);
	void GroupUnits(sc2::Units units);
	sc2::Units													main_army_;
	std::vector <const sc2::Unit*>								defense_queens_;
	std::vector<std::pair<const sc2::Unit*, sc2::Point3D>>		ovi_scout_loc_;
	std::vector<sc2::Point2D>									inital_scout_pos_;
	std::vector<sc2::Point2D>									lingbane_harass_pos_;
	sc2::Units													harass_squad_1;
	sc2::Units													harass_squad_2;
	sc2::Units													muta_squad_1;
	sc2::Units													mutas;
	
	const sc2::Unit*											target;
	const sc2::Unit*											scout;
	bool attack_sent;
	int scout_pos_index;
	bool squad_assigned;
	bool we_are_under_attack;
	bool mutas_in_corner;
	sc2::Point2D escape_corner;
	sc2::Point2D enemy_corner;
	sc2::Point2D muta_attack_pos;
	
	
};

