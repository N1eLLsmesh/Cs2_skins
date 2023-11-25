#pragma once
#include "CCSPlayerPawnBase.h"
#include "schemasystem.h"
//#include "baseviewmodel_shared.h"
//#include "CCSPlayer_ViewModelServices.h"
#include "CEconItemView.h"

class CCSPlayerPawn : public CCSPlayerPawnBase
{
public:
	//SCHEMA_FIELD(CCSPlayer_ViewModelServices*,CCSPlayerPawn, m_pViewModelServices);

	SCHEMA_FIELD(C_EconItemView, CCSPlayerPawn, m_EconGloves);
};
