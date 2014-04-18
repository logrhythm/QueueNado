#include "Death.h"

std::string Death::mMessage
{
   ""
};
bool Death::mReceived(false);


/// @param death message with any captured death details

void Death::Received(g2::internal::FatalMessage death) {
   Death::mReceived = true;
   Death::mMessage = death.message_;
}

bool Death::WasKilled() {
   return Death::mReceived;
}

/// Please call this if you plan on doing DEATH tests. 

void Death::SetupExitHandler() {
   g2::internal::changeFatalInitHandlerForUnitTesting(Death::Received);
}

void Death::ClearExits() {
   Death::mReceived = false;
   Death::mMessage = "";
}

