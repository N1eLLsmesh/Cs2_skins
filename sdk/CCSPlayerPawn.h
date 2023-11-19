#pragma once
#include "CCSPlayerPawnBase.h"
#include "schemasystem.h"
#include "baseviewmodel_shared.h"

class CCSPlayerPawn : public CCSPlayerPawnBase
{
public:
      SCHEMA_FIELD(CbaseViewModel*, CCSPlayerPawn, m_hViewModel);
};
