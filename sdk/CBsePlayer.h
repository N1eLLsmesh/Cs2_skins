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
inline CEntityInstance* UTIL_FindEntityByClassname(CEntityInstance* pStart, const char* name)
{
	extern CEntitySystem* g_pEntitySystem;
	CEntityIdentity* pEntity = pStart ? pStart->m_pEntity->m_pNext : g_pEntitySystem->m_EntityList.m_pFirstActiveEntity;

	for (; pEntity; pEntity = pEntity->m_pNext)
	{
		if (!strcmp(pEntity->m_designerName.String(), name))
			return pEntity->m_pInstance;
	};

	return nullptr;
}

class SC_CBasePlayer : public CBaseEntity
{
public:
	SCHEMA_FIELD(CBasePlayer*, CBaseEntity, m_pPredictionPlayer);
};
