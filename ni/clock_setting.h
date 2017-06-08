#ifndef _CLOCK_SETTING_H_
#define _CLOCK_SETTING_H_

namespace clock_setting {

/*
static const float DEFAULT_BASE_CLOCK_PERIOD_IN_PS = 2500; //2.5 ns

static const unsigned int NUMBER_OF_MODES = 4;
static const float CLOCK_PERIOD_IN_PS_TABLE[ NUMBER_OF_MODES ]
 = { 2500, 5000, 10000, 20000 };
 */
static const float DEFAULT_BASE_CLOCK_PERIOD_IN_PS = 1000; //2.5 ns

static const unsigned int NUMBER_OF_MODES = 4;
static const float CLOCK_PERIOD_IN_PS_TABLE[ NUMBER_OF_MODES ]
 = { 1000, 2000, 4000, 8000 };

static const unsigned int MAX_CLOCK_DIVISOR = 16;

/*
static float getClockPeriodInPsOfMode( unsigned int mode )
{
	if ( mode >= NUMBER_OF_MODES )
		return CLOCK_PERIOD_IN_PS_TABLE[NUMBER_OF_MODES-1];

	return CLOCK_PERIOD_IN_PS_TABLE[mode];
}
*/

}

#endif
