#pragma once
#include "CCSPlayerPawnBase.h"
#include "schemasystem.h"
//#include "baseviewmodel_shared.h"
#include "CCSPlayer_ViewModelServices.h"

class CCSPlayerPawn : public CCSPlayerPawnBase
{
public:
      //SCHEMA_FIELD(CbaseViewModel*, CCSPlayerPawn, m_hViewModel);

      //NETVAR(CCSPlayer_ViewModelServices*, m_pViewModelServices, "client.dll!C_CSPlayerPawnBase->m_pViewModelServices");
	SCHEMA_FIELD(CCSPlayer_ViewModelServices*,CCSPlayerPawn, m_pViewModelServices);
};
