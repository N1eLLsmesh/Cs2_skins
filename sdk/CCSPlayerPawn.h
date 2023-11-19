#pragma once
#include "CCSPlayerPawnBase.h"
#include "schemasystem.h"

class CCSPlayerPawn : public CCSPlayerPawnBase
{
public:
      SCHEMA_FIELD(CbaseViewModel*, CCSPlayerPawn, m_hViewModel);
};
