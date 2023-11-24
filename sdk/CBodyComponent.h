#include "CBodyComponentSkeletonInstance.h"
#include "schemasystem.h"

class CBodyComponent {
public:
    SCHEMA_FIELD(CBodyComponentSkeletonInstance*, CBodyComponent, m_pSceneNode);
};
