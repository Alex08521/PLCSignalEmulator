#ifndef RANDOMIZER_H
#define RANDOMIZER_H

class Randomizer
{
	private:
	int b;
	int c;
	int d;
	int e;
	int iter;
	int prev;
	int prev2;
	int prev3;
	int checker;
	public:
	Randomizer();
	int result(int lim);
};

#endif //RANDOMIZER_H