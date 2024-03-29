#ifndef CALIBRACION_H
#define CALIBRACION_H

#include "config.h"

#define CALIBRACION false

const float selectedVolumeArray[12] = {300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850};

const float pistonDisplacementArray[12] = {24.4, 28.5, 32.6, 36.7, 40.7, 44.8, 48.9, 53.0, 57.0, 61.1, 65.2, 69.3};

const long inspirationCountsArray[12] =
#if (SERIAL_NUMBER == 1)
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
  
#elif (SERIAL_NUMBER == 2)
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

#elif (SERIAL_NUMBER == 3)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 4)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 5)
  {1029,
   1184,
   1315,
   1466,
   1638,
   1802,
   1961,
   2123,
   2273,
   2420,
   2489,
   2600};

#elif (SERIAL_NUMBER == 6)
  {1160,
   1291,
   1459,
   1616,
   1752,
   1910,
   2080,
   2233,
   2394,
   2555,
   2716,
   2870};

#elif (SERIAL_NUMBER == 7)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 8)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 9)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 10)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 11)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 12)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 13)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 14)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 15)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 16)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 17)
  {935,
   1151,
   1290,
   1436,
   1575,
   1743,
   1912,
   2059,
   2220,
   2389,
   2564,
   2780};

#elif (SERIAL_NUMBER == 18)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 19)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 20)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 21)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 22)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 23)
  {979,
   1140,
   1308,
   1456,
   1623,
   1784,
   1963,
   2135,
   2303,
   2485,
   2674,
   2850};

#elif (SERIAL_NUMBER == 24)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 25)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 26)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 27)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 28)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 29)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#elif (SERIAL_NUMBER == 30)
  {1023,
   1194,
   1372,
   1526,
   1686,
   1849,
   2018,
   2176,
   2330,
   2505,
   2670,
   2800};

#endif

#endif