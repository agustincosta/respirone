#ifndef CALIBRACION_H
#define CALIBRACION_H

#include "config.h"

const float selectedVolumeArray[12] = {300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850};

const long inspirationCountsArray[12] =
#if (SERIAL_NUMBER == 001)
  {1081,
  1256,
  1394,
  1547,
  1718,
  1878,
  2042,
  2164,
  2349,
  2502,
  2665,
  2810};
  
#elif (SERIAL_NUMBER == 002)
  {1082,
  1285,
  1408,
  1616,
  1714,
  1875,
  2047,
  2210,
  2370,
  2556,
  2676,
  2780};

#elif (SERIAL_NUMBER == 003)

#elif (SERIAL_NUMBER == 004)

#elif (SERIAL_NUMBER == 005)

#elif (SERIAL_NUMBER == 006)

#elif (SERIAL_NUMBER == 007)

#elif (SERIAL_NUMBER == 008)

#elif (SERIAL_NUMBER == 009)

#elif (SERIAL_NUMBER == 010)

#elif (SERIAL_NUMBER == 011)

#elif (SERIAL_NUMBER == 012)

#elif (SERIAL_NUMBER == 013)

#elif (SERIAL_NUMBER == 014)

#elif (SERIAL_NUMBER == 015)

#elif (SERIAL_NUMBER == 016)

#elif (SERIAL_NUMBER == 017)

#elif (SERIAL_NUMBER == 018)

#elif (SERIAL_NUMBER == 019)

#elif (SERIAL_NUMBER == 020)

#elif (SERIAL_NUMBER == 021)

#elif (SERIAL_NUMBER == 022)

#elif (SERIAL_NUMBER == 023)

#elif (SERIAL_NUMBER == 024)

#elif (SERIAL_NUMBER == 025)

#elif (SERIAL_NUMBER == 026)

#elif (SERIAL_NUMBER == 027)

#elif (SERIAL_NUMBER == 028)

#elif (SERIAL_NUMBER == 029)

#elif (SERIAL_NUMBER == 030)

#endif

#endif