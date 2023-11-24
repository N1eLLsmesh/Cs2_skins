#include "schemasystem.h"
#include "baseentity.h"
//#include "CBaseViewModel.h"

//#include "CBodyComponent.h"
#include "CPlayer_ItemServices.h"
#include "CBodyComponentSkeletonInstance.h"
class CBaseViewModel;
class C_CSGOViewModel /*: public CBaseViewModel*/ {
public:
    SCHEMA_FIELD(CBodyComponent*, C_BaseEntity, m_CBodyComponent);
    SCHEMA_FIELD(CBodyComponentSkeletonInstance*, C_BaseEntity, m_pGameSceneNode);

    //SCHEMA_FIELD(Vector3, C_BaseFlex, m_CachedViewTarget);
    SCHEMA_FIELD(CHandle<void*>, CBaseViewModel, m_iCameraAttachment);
};
