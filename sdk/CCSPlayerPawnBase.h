#pragma once
#include "CBasePlayerPawn.h"
#include "schemasystem.h"
#include "CCSPlayer_ViewModelServices.h"

class C_CSPlayerPawnBase;
class CCSPlayerPawnBase : public CBasePlayerPawn
{
public:
	SCHEMA_FIELD(int32_t, CCSPlayerPawnBase, m_ArmorValue);
	//SCHEMA_FIELD(CHandle<C_EconItemView*>,CCSPlayerPawnBase,m_EconGloves);
	SCHEMA_FIELD_CLIENT(CCSPlayer_ViewModelServices*,C_CSPlayerPawnBase, m_pViewModelServices);
};	
