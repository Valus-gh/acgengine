/**
 * @file		engine_timer.cpp
 * @brief   Timing utils
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // Windows:
   #include <Windows.h>



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief Timer reserved structure.
 */
struct Eng::Timer::Reserved
{
   uint64_t cpuFreq;   ///< Frequency multiplier used by performance counter


   /**
    * Constructor.
    */
   Reserved() : cpuFreq{ 0 }
   {
      if (!QueryPerformanceFrequency((LARGE_INTEGER *)&cpuFreq))
         ENG_LOG_ERROR("Performance counter not supported");
   }
};



/////////////////////////
// BODY OF CLASS Timer //
/////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::Timer::Timer() : reserved(std::make_unique<Eng::Timer::Reserved>())
{
   ENG_LOG_DEBUG("[+]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::Timer::~Timer()
{
   ENG_LOG_DEBUG("[-]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Get singleton instance.
 */
Eng::Timer ENG_API &Eng::Timer::getInstance()
{
   static Timer instance;
   return instance;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Get current time elapsed in ticks.
 * @return Time elapsed in ticks
 */
uint64_t ENG_API Eng::Timer::getCounter() const
{
   // Not supported?
   if (reserved->cpuFreq == 0)
      return 0;

   uint64_t li = 0;
   QueryPerformanceCounter((LARGE_INTEGER *)&li);
   return li;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Get time interval in milliseconds between two tick snapshots.
 * @return Time elapsed in milliseconds
 */
double ENG_API Eng::Timer::getCounterDiff(uint64_t t1, uint64_t t2) const
{
   // Not supported?
   if (reserved->cpuFreq == 0)
      return 0.0;

   uint64_t r = ((t2 - t1) * 1000000) / reserved->cpuFreq;

   return static_cast<double>(r) / 1000.0;
}
