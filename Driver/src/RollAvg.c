
#include "RollAvg.h"

void RollAvg( uint32_t  DATA[], double res[])
{
  double y;
  int b_k;
  int iLeft;
  int k;
  int vlen;
  for (k = 0; k < 100; k++) 
	{
			if (k + 1 <= 5) 
			{
				iLeft = -5;
			} 
			else 
			{
				iLeft = k - 10;
			}
			if (k + 5 > 100) 
			{
				b_k = 95;
			} 
			else 
			{
				b_k = k;
			}
			vlen = b_k - iLeft;
			if (vlen == 0) 
			{
				y = 0.0;
			} 
			else 
			{
				y = DATA[iLeft + 5];
				for (b_k = 2; b_k <= vlen; b_k++) 
				{
						y += (double)DATA[(iLeft + b_k) + 4];
				}
			}
			res[k] = y / (double)vlen;
  }
}


