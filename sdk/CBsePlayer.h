#pragma once
#include "c_basecombatcharacter.h"

//class C_BaseViewModel;

class C_BaseViewModel : public C_BaseCombatCharacter
{
      
public:
     // DECLARE_CLASS( C_BasePlayer, C_BaseCombatCharacter );
      //C_BasePlayer();
	    //virtual			~C_BasePlayer();
      C_BaseViewModel		*GetViewModel( int viewmodelindex = 0 );
}
