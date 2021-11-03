// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  static bool_t HooksEnabled = False;
  static Map<oCNpc*, uint> NpcsNoFocus;
  static Array<uint> NeverFocusInstances;
  static Array<uint> AlwaysFocusInstances;


  static void HoldNpcsNoFocus( oCNpc* npc ) {
    auto& pair = NpcsNoFocus[npc];
    if( pair.IsNull() )
      return;

    NpcsNoFocus.Insert( npc, Timer::GetTime() );
  }


  static bool CheckNpcsNoFocus( oCNpc* npc ) {
    auto& pair = NpcsNoFocus[npc];
    if( pair.IsNull() )
      return true;

    if( Timer::GetTime() - pair.GetValue() > 2000 ) {
      NpcsNoFocus.Remove( npc );
      return true;
    }

    return false;
  }


  static int GetPartyMemberID() {
    zCPar_Symbol* sym = parser->GetSymbol( "AIV_PARTYMEMBER" );
    if( !sym )
#if ENGINE >= Engine_G2
      return 15;
#else
      return 36;
#endif
    
    int id;
    sym->GetValue( id, 0 );
    return id;
  }


  bool oCNpc::IsEnemy( oCNpc* npc ) {
    if( npc == this )
      return false;

    return npc && GetAttitude( npc ) <= NPC_ATT_ANGRY;
  }


  bool oCNpc::IsFightStatus() {
    return oCZoneMusic::s_herostatus != oHERO_STATUS_STD;
  }


  bool oCNpc::IsFriendKiller( oCNpc* npc ) {
    return npc && npc->enemy && (npc->enemy == player || !IsEnemy( npc->enemy ));
  }


  bool oCNpc::WantToKillSomebody() {
    return enemy && state.curState.valid;
  }


  bool oCNpc::IsInFightMode() {
    return fmode != 0;
  }


  bool oCNpc::IsPartyMember() {
    static int AIV_PARTYMEMBER = GetPartyMemberID();
    return aiscriptvars[AIV_PARTYMEMBER] != False && enemy != player;
  }


  bool oCNpc::PlayerCanDamage( oCNpc* npc ) {
    if( !IsEnemy( npc ) ) {
      if( npc->IsPartyMember() )
        return false;

      if( IsEnemy( npc->enemy ) && npc->IsInFightMode() )
        return false;

      if( IsFightStatus() && !npc->IsInFightMode() )
        return false;
    }

    return true;
  }


  bool oCNpc::CanDamage( oCNpc* npc ) {
    if( NeverFocusInstances.HasEqualSorted( npc->instanz ) && npc->enemy != player )
      return false;

    if( AlwaysFocusInstances.HasEqualSorted( npc->instanz ) )
      return true;

    if( !npc )
      return true;

    if( this != player ) {
      // Exclude attacker from nofocus list
      if( IsInFightMode() && enemy == player )
        NpcsNoFocus.Remove( this );

      // Anti-friendly-AOE
      if( npc == player && enemy != npc && !IsEnemy( npc ) )
        return false;

      return true;
    }

    if( !PlayerCanDamage( npc ) )
      NpcsNoFocus.Insert( npc, Timer::GetTime() );

    return CheckNpcsNoFocus( npc );
  }


  bool oCNpc::CanDamage( oSDamageDescriptor& desc ) {
    return !desc.pNpcAttacker || desc.pNpcAttacker->CanDamage( this );
  }


  HOOK Hook_oCNpc_OnDamage PATCH_IF( &oCNpc::OnDamage, &oCNpc::OnDamage_Union, false );

  void oCNpc::OnDamage_Union( oSDamageDescriptor& desc ) {
    HoldNpcsNoFocus( this );
    if( CanDamage( desc ) )
      THISCALL( Hook_oCNpc_OnDamage )(desc);
  }


  static oCNpc* s_RootNpc = Null;

  HOOK Hook_oCNpc_CreateVobList PATCH_IF( &oCNpc::CreateVobList, &oCNpc::CreateVobList_Union, false );

  void oCNpc::CreateVobList_Union( float dist ) {
    s_RootNpc = this;
    THISCALL( Hook_oCNpc_CreateVobList )(dist);
    s_RootNpc = Null;
  }


  HOOK Hook_oCNpc_CreateVobList_Array PATCH_IF( &oCNpc::CreateVobList, &oCNpc::CreateVobList_Array_Union, false );

  void oCNpc::CreateVobList_Array_Union( zCArray<zCVob*>& vobList, float dist ) {
    s_RootNpc = this;
    THISCALL( Hook_oCNpc_CreateVobList_Array )(vobList, dist);
    s_RootNpc = Null;
  }


  HOOK Hook_oCNpc_GetNearestValidVob PATCH_IF( &oCNpc::GetNearestValidVob, &oCNpc::GetNearestValidVob_Union, false );

  void oCNpc::GetNearestValidVob_Union( float dist ) {
    s_RootNpc = this;
    THISCALL( Hook_oCNpc_GetNearestValidVob )(dist);
    s_RootNpc = Null;
  }


  HOOK Hook_oCNpc_GetNearestVob AS_IF( &oCNpc::GetNearestVob, &oCNpc::GetNearestVob_Union, false );

  void oCNpc::GetNearestVob_Union( float max_dist ) {
    s_RootNpc = this;
    THISCALL( Hook_oCNpc_GetNearestVob )(max_dist);
    s_RootNpc = Null;
  }


  HOOK Hook_zCBspBase_CollectVobsInBBox3D PATCH_IF( &zCBspBase::CollectVobsInBBox3D, &zCBspBase::CollectVobsInBBox3D_Union, false );

  void zCBspBase::CollectVobsInBBox3D_Union( zCArray<zCVob*>& vobList, const zTBBox3D& bbox ) const {
    THISCALL( Hook_zCBspBase_CollectVobsInBBox3D )(vobList, bbox);

    if( !s_RootNpc )
      return;

    if( s_RootNpc == player ) {
      if( s_RootNpc->fmode != NPC_WEAPON_NONE ) {
        for( int i = 0; i < vobList.GetNum(); i++ ) {
          oCNpc* npc = vobList[i]->CastTo<oCNpc>();
          if( npc && !player->CanDamage( npc ) )
            vobList.RemoveIndex( i-- );
        }
      }
      else
        NpcsNoFocus.Clear();
    }
  }


  void EnableHooks() {
    Hook_oCNpc_OnDamage.Commit();
    Hook_oCNpc_CreateVobList.Commit();
    Hook_oCNpc_CreateVobList_Array.Commit();
    Hook_oCNpc_GetNearestValidVob.Commit();
    Hook_oCNpc_GetNearestVob.Commit();
    Hook_zCBspBase_CollectVobsInBBox3D.Commit();
  }


  void DisableHooks() {
    Hook_oCNpc_OnDamage.Detach();
    Hook_oCNpc_CreateVobList.Detach();
    Hook_oCNpc_CreateVobList_Array.Detach();
    Hook_oCNpc_GetNearestValidVob.Detach();
    Hook_oCNpc_GetNearestVob.Detach();
    Hook_zCBspBase_CollectVobsInBBox3D.Detach();
  }


  void UpdateOptions() {
    int enabled = zoptions->ReadBool( "zNoFriendlyFire", "Enabled", True );
    if( enabled != HooksEnabled ) {
      HooksEnabled = enabled;
      if( HooksEnabled )
        EnableHooks();
      else
        DisableHooks();
    }
  }
}