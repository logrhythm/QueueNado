#pragma once
#include <string>

#ifndef _VIRTUAL
#ifdef _DEBUG
#define _VIRTUAL virtual
#else
#define _VIRTUAL
#endif
#endif

#ifndef _OVERRIDE
#ifdef _DEBUG
#define _OVERRIDE override
#else
#define _OVERRIDE
#endif
#endif

#ifndef _FINAL
#ifdef _DEBUG
#define _FINAL 
#else
#define _FINAL final
#endif
#endif

#ifdef _DEBUG
#ifndef RIFLE_VAMPIRE_PRODUCTION
#define RIFLE_VAMPIRE_PRODUCTION (0)
#endif
#else
// ref: ProbeReader/ProbeManager/ProbeLogger.cpp and g2log/g2loglevels.hpp
#define G2_DYNAMIC_LOGGING
#ifndef RIFLE_VAMPIRE_PRODUCTION
#define RIFLE_VAMPIRE_PRODUCTION (1)
#endif
#endif

#define KB_TO_MB_SHIFT (10)
#define GB_TO_MB_SHIFT (10)
#define MB_TO_GB_SHIFT (10)
#define MB_TO_TB_SHIFT (20)
#define TB_TO_MB_SHIFT (20)
#define GB_TO_KB_SHIFT (20)
#define GB_TO_TB_SHIFT (10)
#define MB_TO_KB_SHIFT (10)
#define KB_TO_GB_SHIFT (20)
#define B_TO_KB_SHIFT (10)
#define B_TO_MB_SHIFT (20)
#define B_TO_GB_SHIFT (30)
#define B_TO_TB_SHIFT (40)
#define TB_TO_GB_SHIFT (10)
#define TB_TO_KB_SHIFT (30)
#define KB_TO_TB_SHIFT (30)
#define KB_TO_B_SHIFT (10)
#define MB_TO_B_SHIFT (20)
#define GB_TO_B_SHIFT (30)
#define TB_TO_B_SHIFT (40)
#define ETH_ADDR_SIZE (6) 
#define IPV6_NUM_QUADS (8) 
#define MINUTES_TO_SECONDS (60)
#define HOURS_TO_MINUTES (60)
#define HOURS_TO_SECONDS (MINUTES_TO_SECONDS*HOURS_TO_MINUTES)
#define MILLISECONDS_TO_SECONDS (1000)

#define UUID_SIZE (36)

