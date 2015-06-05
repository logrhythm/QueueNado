#pragma once
#include <string>

#ifndef LR_VIRTUAL
#ifdef LR_DEBUG
#define LR_VIRTUAL virtual
#else
#define LR_VIRTUAL
#endif
#endif

#ifndef LR_OVERRIDE
#ifdef LR_DEBUG
#define LR_OVERRIDE override
#else
#define LR_OVERRIDE
#endif
#endif

#ifndef LR_FINAL
#ifdef LR_DEBUG
#define LR_FINAL 
#else
#define LR_FINAL final
#endif
#endif

#ifdef LR_DEBUG
#ifndef RIFLE_VAMPIRE_PRODUCTION
#define RIFLE_VAMPIRE_PRODUCTION (0)
#endif
#else
#ifndef RIFLE_VAMPIRE_PRODUCTION
#define RIFLE_VAMPIRE_PRODUCTION (1)
#endif
#endif

#ifndef MINUTES_TO_SECONDS
#define MINUTES_TO_SECONDS (60)
#endif

