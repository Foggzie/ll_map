// Include libraries
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "ll_map.h"
 
int main(int argc, char* argv[])
{
	ll_map burlington_map;

	
	burlington_map.build_map(
		44.476221,	// Latitude
		-73.205595,	// Longitude
		1000.00,	// Width
		20);		// Density
}
