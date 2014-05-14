/* 
 * Author: Robert Weber
 *
 * Created on November 7, 2012, 2:35 PM
 */
#pragma once

#include "Zombie.h"

class MockZombie : public Zombie {
public:
   MockZombie(std::string& binding) : Zombie(binding), mBulletHoles(0) {}
   ~MockZombie();
   
   int mBulletHoles;
};

