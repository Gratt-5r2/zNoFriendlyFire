// Supported with union (c) 2020 Union team

// User API for oCNpc
// Add your methods here

void OnDamage_Union( oSDamageDescriptor& );
void OnDamage_Hit_Union( oSDamageDescriptor& );
void OnDamage_Sound_Union( oSDamageDescriptor& );
bool PlayerCanDamage( oCNpc* npc );
bool CanDamage( oSDamageDescriptor& );
bool CanDamage( oCNpc* npc );
void CreateVobList_Union( float max_dist );
void GetNearestValidVob_Union( float max_dist );
void GetNearestVob_Union( float max_dist );
void CreateVobList_Array_Union( zCArray<zCVob*>&, float );
bool IsEnemy( oCNpc* npc );
bool IsFightStatus();
bool IsFriendKiller( oCNpc* npc );
bool WantToKillSomebody();
bool IsInFightMode();
bool IsPartyMember();