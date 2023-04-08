#include "Randomizer.h"

Randomizer::Randomizer()
	{
		iter = 1;
		prev = 0;
		prev2 = 0;
		prev3 = 0;
		checker = 1;
		int *vb = new int;
		int *vc = new int;
		int *vd = new int;
		int *ve = new int;
		b = *vb;
		c = *vc;
		d = *vd;
		e = *ve;
		delete vb;
		delete vc;
		delete vd;
		delete ve;
	}
	
	
	int Randomizer::result(int lim)
	{
		int result = (((b+c+d+e)/iter)%lim)+1;
		if (prev == result)
		{
			bool done = false;
			while(done == false)
			{	
				int *vvar = new int;
				int var = *vvar;
				result = (((b+c)/d*e+(iter+prev)+var)%lim)+1;
				if (result != prev) done = true;
				var++;
				delete vvar;
			}
		}
		if (prev2 == result)
		{
			bool done = false;
			while(done == false)
			{	
				int *vvar = new int;
				int var = *vvar;
				result = (((b+c)/d*e+(iter+prev)+var)%lim)+1;
				if (result != prev) done = true;
				var++;
				delete vvar;
			}
		}
		if (prev3 == result)
		{
			bool done = false;
			while(done == false)
			{	
				int *vvar = new int;
				int var = *vvar;
				result = (((b+c)/d*e+(iter+prev)+var)%lim)+1;
				if (result != prev) done = true;
				var++;
				delete vvar;
			}
		}
		switch (checker)
		{
			case 1:
				prev = result;
				checker = 2;
				break;
			case 2:
				prev2 = result;
				checker = 3;
				break;
			case 3:
				prev3 = result;
				checker = 1;
				break;
		}
		iter++;
		return (result > (-1) ? result : result*(-1));
	}
