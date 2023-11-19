#pragma once
#include <entity2/entityidentity.h>
#include <baseentity.h>
#include "schemasystem.h"
#include "ehandle.h"

//#include "playernet_vars.h"
#include "shared_classnames.h"
//#include "CBsePlayer.h"
//#include "c_baseplayer.h"

//class CBasePlayer*;

class SC_CBasePlayer : public CBaseEntity
{
public:
	SCHEMA_FIELD(CBasePlayer*, CBaseEntity, m_pPredictionPlayer);
};
