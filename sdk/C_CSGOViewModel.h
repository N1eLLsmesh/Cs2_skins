#include "schemasystem.h"
#include "baseentity.h"
#include "CBaseFlex.h"
#include "CBaseViewModel.h"

//#include "CBodyComponent.h"
#include "CPlayer_ItemServices.h"
#include "CBodyComponentSkeletonInstance.h"


struct Vector3 {
	float x, y, z;
};

class CNetworkViewOffsetVector {
    float X; // CNetworkedQuantizedFloat
    float Y; // CNetworkedQuantizedFloat
    float Z; // CNetworkedQuantizedFloat
}
class CBaseModelEntity;
class CCSGOViewModel /*: public CBaseViewModel*/ {
public:
    SCHEMA_FIELD(CBodyComponent*, C_BaseEntity, m_CBodyComponent);
    SCHEMA_FIELD(CBodyComponentSkeletonInstance*, C_BaseEntity, m_pGameSceneNode);
	//m_vecLastFacing CBaseViewModel
    SCHEMA_FIELD(CNetworkViewOffsetVector, CBaseModelEntity, m_vecViewOffset);
    SCHEMA_FIELD(Vector3, CBaseFlex, m_CachedViewTarget);
    SCHEMA_FIELD(CHandle<void*>, CBaseViewModel, m_iCameraAttachment);
};
