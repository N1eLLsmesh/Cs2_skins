#include "CModelState.h"
#include "schemasystem.h"

class CSkeletonInstance {
public:
	//NETVAR(CModelState, m_modelState, "client.dll!CSkeletonInstance->m_modelState");
  SCHEMA_FIELD(CModelState, CSkeletonInstance, m_modelState);
};
