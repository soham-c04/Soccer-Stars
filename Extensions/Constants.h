#pragma once
#include <graphics.h>
#include <math.h>

extern const double pi;
double rad(int degree);
double deg(double radi);

#define ms 1000
#define Xratio X_full/1366  // For Adjusting to size of objects in X direction according to resolution of Screen
#define Yratio Y_full/768 	// For Adjusting to size of objects in Y direction according to resolution of Screen
#define Cratio 1 			// For Adjusting to size of objects in circular direction according to resolution of Screen
// Initial build resolution of PC - 1366 * 768

extern int width, height;
extern const int X_full, Y_full;
extern int X1, Y1, X2, Y2, X, Y;

extern const double e;
extern const int DELAY, MAX_COL;
