// ll_map
// Longitude & Latitude Map

// NOTES:
// 1 degree = 69 miles
// Burlington
// Lat: 44.476266
// Lon: -73.205509

#ifndef LL_MAP_H
#define LL_MAP_H

#include <iostream>
#include <math.h>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct my_string {
	char *ptr;
	size_t len;
};
class ll_map;

void init_my_string(my_string*);
size_t write_func(void*, size_t, size_t, my_string*);
void pull_data(double*, double*, size_t num_points);

class ll_map {
private:
	// The geographic center of the map
	double m_latitude;
	double m_longitude;

	// The distance between two adjacent map points
	double m_spacing_degrees;
	double m_spacing_meters;

	// The upper left of the map
	double m_upper_left_lat;
	double m_upper_left_lon;
	
	// How far the map spans from the center
	double m_width_degrees;
	double m_width_meters;

	// The density of the map
	size_t m_density;

	// The height map
	float** m_map;

public:
	ll_map();
	~ll_map();

	void build_map(double, double, double, size_t);
	void clean_map();
	void from_int_to_xy(size_t, size_t*, size_t*);
	void from_ll_to_xy(double, double, size_t*, size_t*);
	void from_xy_to_int(size_t, size_t, size_t*);
	void from_xy_to_ll(size_t, size_t, double*, double*);
};

#endif
