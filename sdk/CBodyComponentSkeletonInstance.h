#include "CSkeletonInstance.h"
#include "schemasystem.h"

class CBodyComponentSkeletonInstance {
public:
    SCHEMA_FIELD(CSkeletonInstance, CBodyComponentSkeletonInstance, m_skeletonInstance);
    SCHEMA_FIELD(uint64_t, CBodyComponentSkeletonInstance, m_iMeshGroupMaskMain);
};
