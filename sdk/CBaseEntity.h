#pragma once
#include <entity2/entityidentity.h>
#include <baseentity.h>
#include "schemasystem.h"
#include "ehandle.h"
#include "CBsePlayer.h"

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

class SC_CBaseEntity : public CBaseEntity
{
public:
    SCHEMA_FIELD(int32_t, CBaseEntity, m_iHealth);
    SCHEMA_FIELD(int32_t, CBaseEntity, m_iMaxHealth);
    SCHEMA_FIELD(LifeState_t, CBaseEntity, m_lifeState);
    SCHEMA_FIELD(uint8_t, CBaseEntity, m_iTeamNum);
    SCHEMA_FIELD(float, CBaseEntity, m_flGravityScale);
    SCHEMA_FIELD(CHandle<SC_ViewModel>, CBaseViewModel, m_hViewModel);

//CBaseViewModel* ToBaseViewModel()
  //  {
      //  if ( !m_hViewModel )
     //       return nullptr;
        
     //   return ::ToBaseViewModel(m_hViewModel.Get());
   // }

    //CBasePlayer* GetPlayer();
   // virtual CBasePlayer* GetPredictionOwner();  // Добавьте это определение
};
class CCSPlayer_ViewModelServices : public CPlayer_ViewModelServices
{
public:
 // MNetworkEnable
 CHandle< CBaseViewModel > m_hViewModel[3]; // 0x40 
};
//class SC_CBasePlayer : public CBaseEntity
//{
//public:
//	SCHEMA_FIELD(CBasePlayer*, CBaseEntity, m_pPredictionPlayer);
//};

//CBasePlayer* GetPlayerFromEntity(SC_CBaseEntity* entity);

//// Реализация метода для получения указателя на CBasePlayer
//CBasePlayer* GetPlayerFromEntity(SC_CBaseEntity* entity)
//{
//    return dynamic_cast<CBasePlayer*>(entity->GetPredictionOwner());
//}

//// Реализация метода для получения указателя на CBasePlayer из SC_CBaseEntity
//CBasePlayer* SC_CBaseEntity::GetPlayer()
//{
//    return GetPlayerFromEntity(this);
//}
