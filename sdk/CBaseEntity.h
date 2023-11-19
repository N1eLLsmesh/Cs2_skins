#pragma once
#include <entity2/entityidentity.h>
#include <baseentity.h>
#include "schemasystem.h"
#include "ehandle.h"

//#include "playernet_vars.h"
#include "shared_classnames.h"
//#include "c_baseplayer.h"

class CBasePlayer;

class SC_CBaseEntity : public CBaseEntity
{
public:
    SCHEMA_FIELD(int32_t, CBaseEntity, m_iHealth);
    SCHEMA_FIELD(int32_t, CBaseEntity, m_iMaxHealth);
    SCHEMA_FIELD(LifeState_t, CBaseEntity, m_lifeState);
    SCHEMA_FIELD(uint8_t, CBaseEntity, m_iTeamNum);
    SCHEMA_FIELD(float, CBaseEntity, m_flGravityScale);

    CBasePlayer* GetPlayer();
    virtual CBasePlayer* GetPredictionOwner();  // Добавьте это определение
};

CBasePlayer* GetPlayerFromEntity(SC_CBaseEntity* entity);

// Реализация метода для получения указателя на CBasePlayer
CBasePlayer* GetPlayerFromEntity(SC_CBaseEntity* entity)
{
    return dynamic_cast<CBasePlayer*>(entity->GetPredictionOwner());
}

// Реализация метода для получения указателя на CBasePlayer из SC_CBaseEntity
CBasePlayer* SC_CBaseEntity::GetPlayer()
{
    return GetPlayerFromEntity(this);
}
