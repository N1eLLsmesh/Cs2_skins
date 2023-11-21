#pragma once
#include "CBaseCombatCharacter.h"
#include "CPlayer_ItemServices.h"
#include "CBasePlayerController.h"
#include "ehandle.h"
#include "schemasystem.h"

class CBasePlayerPawn : public CBaseCombatCharacter
{
public:
	SCHEMA_FIELD(CPlayer_WeaponServices*, CBasePlayerPawn, m_pWeaponServices);
	SCHEMA_FIELD(CPlayer_ItemServices*, CBasePlayerPawn, m_pItemServices);
	SCHEMA_FIELD(CHandle<CBasePlayerController>, CBasePlayerPawn, m_hController);
};

// Alignment: 136
// Size: 0x1550
class CCSPlayerPawnBase : public CBasePlayerPawn
{
 // ...
 CPlayer_ViewModelServices* m_pViewModelServices; // 0xbc0 
 // ...
};
