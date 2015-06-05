#pragma once
#include <string>

#ifndef QN_VIRTUAL
#ifdef QN_DEBUG
#define QN_VIRTUAL virtual
#else
#define QN_VIRTUAL
#endif
#endif

#ifndef QN_OVERRIDE
#ifdef QN_DEBUG
#define QN_OVERRIDE override
#else
#define QN_OVERRIDE
#endif
#endif

#ifndef QN_FINAL
#ifdef QN_DEBUG
#define QN_FINAL 
#else
#define QN_FINAL final
#endif
#endif

#ifdef QN_DEBUG
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

