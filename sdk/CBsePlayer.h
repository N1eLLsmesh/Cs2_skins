#pragma once
//#include <entity2/entityidentity.h>
//#include <baseentity.h>
#include "schemasystem.h"
#include "ehandle.h"
//#include "shared_classnames.h"

//#include <c_baseviewmodel.h>


//class SC_ViewModel : public CBaseViewModel
//{
//public:
/	//SCHEMA_FIELD(CHandle<CBasePlayer>, CBaseEntity, m_pPredictionPlayer);
//};


class SC_ViewModel
{
public:
	SCHEMA_FIELD(CHandle<SC_ViewModel>, CBaseViewModel, m_hViewModel);
	//SCHEMA_FIELD(int32_t, CCSPlayerController_InGameMoneyServices, m_iStartAccount);
};
