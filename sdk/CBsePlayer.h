#pragma once
#include "CBaseCombatCharacter.h"

//class C_BaseViewModel;

class C_BaseViewModel : public CBaseCombatCharacter
{
      
public:
     // DECLARE_CLASS( C_BasePlayer, C_BaseCombatCharacter );
      //C_BasePlayer();
	    //virtual			~C_BasePlayer();
      C_BaseViewModel		*GetViewModel( int viewmodelindex = 0 );
}
