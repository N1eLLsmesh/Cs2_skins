#include "C_CSGOViewModel.h"
#include "schemasystem.h"

class CCSPlayer_ViewModelServices {
public:
	//NETVAR(CHandle<C_CSGOViewModel>, m_hViewModel, "client.dll!CCSPlayer_ViewModelServices->m_hViewModel")
  //SCHEMA_FIELD(CHandle<C_CSGOViewModel>,);
  SCHEMA_FIELD(CHandle<C_CSGOViewModel>, CCSPlayer_ViewModelServices, m_hViewModel);

};