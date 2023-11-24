#include "schemasystem.h"
#include "baseentity.h"

class C_CSGOViewModel: public C_BaseViewModel {
public:
    SCHEMA_FIELD(CBodyComponent*, C_BaseEntity, m_CBodyComponent);
    SCHEMA_FIELD(CBodyComponentSkeletonInstance*, C_BaseEntity, m_pGameSceneNode);

    SCHEMA_FIELD(Vector3, C_BaseFlex, m_CachedViewTarget);
    SCHEMA_FIELD(CHandle<void*>, C_BaseViewModel, m_iCameraAttachment);
};
