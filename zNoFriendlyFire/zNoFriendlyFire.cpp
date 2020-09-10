// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  Map<oCNpc*, uint> NpcsNoFocus;

  static bool CheckNpcsNoFocus( oCNpc* npc ) {
    auto& pair = NpcsNoFocus[npc];
    if( pair.IsNull() )
      return true;

    if( Timer::GetTime() - pair.GetValue() > 2500 ) {
      NpcsNoFocus.Remove( npc );
      return true;
    }

    return false;
  }



  bool oCNpc::CanDamage( oCNpc* npc ) {
    bool IsFightSound   = oCZoneMusic::s_herostatus != oHERO_STATUS_STD;
    bool IsEnemy        = GetAttitude( npc ) <= NPC_ATT_ANGRY;
    bool IsFriendKiller = enemy && (enemy == player || enemy->GetAttitude( player ) >= NPC_ATT_NEUTRAL);

    if( npc == player && aiscriptvars[15] )
      return false;

    if( !enemy ) {
      if( !IsFightSound )
        return CheckNpcsNoFocus( this );
    }
    else
      NpcsNoFocus.Insert( this, Timer::GetTime() );

    if( IsFriendKiller ) {
      NpcsNoFocus.Remove( this );
      return true;
    }

    if( IsEnemy ) {
      NpcsNoFocus.Remove( this );
      return true;
    }

    return CheckNpcsNoFocus( this );
  }



  bool oCNpc::CanDamage( oSDamageDescriptor& desc ) {
    return CanDamage( desc.pNpcAttacker );
  }



  HOOK Hook_oCNpc_OnDamage AS( &oCNpc::OnDamage, &oCNpc::OnDamage_Union );

  void oCNpc::OnDamage_Union( oSDamageDescriptor& desc ) {
    if( CanDamage( desc ) )
      THISCALL( Hook_oCNpc_OnDamage )(desc);
  }



  HOOK Hook_oCNpc_OnDamage_Hit AS( &oCNpc::OnDamage_Hit, &oCNpc::OnDamage_Hit_Union );

  void oCNpc::OnDamage_Hit_Union( oSDamageDescriptor& desc ) {
    if( CanDamage( desc ) )
      THISCALL( Hook_oCNpc_OnDamage_Hit )(desc);
  }



  HOOK Hook_oCNpc_OnDamage_Sound AS( &oCNpc::OnDamage_Sound, &oCNpc::OnDamage_Sound_Union );

  void oCNpc::OnDamage_Sound_Union( oSDamageDescriptor& desc ) {
    if( CanDamage( desc ) )
      THISCALL( Hook_oCNpc_OnDamage_Sound )(desc);
  }







  static oCNpc* s_RootNpc = Null;

  HOOK Hook_oCNpc_CreateVobList AS( &oCNpc::CreateVobList, &oCNpc::CreateVobList_Union );

  void oCNpc::CreateVobList_Union( float dist ) {
    s_RootNpc = this;
    THISCALL( Hook_oCNpc_CreateVobList )(dist);
    s_RootNpc = Null;
  }



  HOOK Hook_oCNpc_CreateVobList_Array AS( &oCNpc::CreateVobList, &oCNpc::CreateVobList_Array_Union );

  void oCNpc::CreateVobList_Array_Union( zCArray<zCVob*>& vobList, float dist ) {
    s_RootNpc = this;
    THISCALL( Hook_oCNpc_CreateVobList_Array )(vobList, dist);
    s_RootNpc = Null;
  }



  HOOK Hook_oCNpc_GetNearestValidVob AS( &oCNpc::GetNearestValidVob, &oCNpc::GetNearestValidVob_Union );

  void oCNpc::GetNearestValidVob_Union( float dist ) {
    s_RootNpc = this;
    THISCALL( Hook_oCNpc_GetNearestValidVob )(dist);
    s_RootNpc = Null;
  }



  HOOK Hook_oCNpc_GetNearestVob AS( &oCNpc::GetNearestVob, &oCNpc::GetNearestVob_Union );

  void oCNpc::GetNearestVob_Union( float max_dist ) {
    s_RootNpc = this;
    THISCALL( Hook_oCNpc_GetNearestVob )(max_dist);
    s_RootNpc = Null;
  }



  HOOK Hook_zCBspBase_CollectVobsInBBox3D AS( &zCBspBase::CollectVobsInBBox3D, &zCBspBase::CollectVobsInBBox3D_Union );

  void zCBspBase::CollectVobsInBBox3D_Union( zCArray<zCVob*>& vobList, const zTBBox3D& bbox ) const {
    THISCALL( Hook_zCBspBase_CollectVobsInBBox3D )(vobList, bbox);

    if( !s_RootNpc )
      return;

    if( s_RootNpc == player ) {
      if( s_RootNpc->fmode != NPC_WEAPON_NONE ) {
        for( int i = 0; i < vobList.GetNum(); i++ ) {
          oCNpc* npc = vobList[i]->CastTo<oCNpc>();
          if( npc && !npc->CanDamage( player ) )
            vobList.RemoveIndex( i-- );
        }
      }
      else
        NpcsNoFocus.Clear();
    }
  }
}