// AxH.cpp : Defines the entry point for the application.
//

#include "AxH.h"
#include "math.h"

using namespace std;

int main() {
	math::vector<double, 4> vect = { 1, 1, 1, 1 };
	math::vector<double, 4> vect2 = { 1, 1, 1, 1 };
	math::matrix<double, 4, 4> mat = { {1,0,0,1}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} };

	vect2 = mat * vect;

	return 0;
}
