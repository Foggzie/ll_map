#include "ll_map.h"

namespace gfox
{

void init_my_string(my_string *s) {
	s->len = 0;
	s->ptr = (char*)malloc(s->len+1);
	if (s->ptr == NULL) {
		// Throw custome exception
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

size_t write_func(void *ptr, size_t size, size_t nmemb, my_string *s) {
	size_t new_len = s->len + size*nmemb;
	s->ptr = (char*)realloc(s->ptr, new_len+1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

void pull_data(double* ll, double* heights, size_t num_points) {
	std::string elevUrl= "http://open.mapquestapi.com/elevation/v1/getElevationProfile?shapeFormat=raw&outFormat=xml&latLngCollection=";
 	
	
	// Add longitudes and latitudes to the string
	for (size_t i = 0; i < num_points*2; i+=2) {
		std::stringstream ss;
		//ss.precision(10);
		ss << ll[i] << "," << ll[i+1];

		if (i+2 < num_points*2)
			ss << ",";

		elevUrl += ss.str();
	}

	std::cout << elevUrl;

	// Global init curl
	curl_global_init(CURL_GLOBAL_ALL);
	char* curlErrStr = (char*)malloc(CURL_ERROR_SIZE);
	
	// Create a curl handle
	CURL* curlHandle = curl_easy_init();
	
	if (curlHandle) {
		// Create a 'my_string' to hold the xml data
		my_string* xml_data = (my_string*)malloc(sizeof(my_string));

		init_my_string(xml_data);

		// Set curl options
		curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, curlErrStr);
		curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, xml_data);
		curl_easy_setopt(curlHandle, CURLOPT_URL, elevUrl.c_str());

		// Work that libcurl magic
		CURLcode curlErr = curl_easy_perform(curlHandle);
		//printf("\n%s\n", xml_data->ptr);

		// Where there any errors with libcurl
		if (curlErr) {
			printf("%s\n", curl_easy_strerror(curlErr));
		}
		// Print the heights
		else {
			// char-by-char
			size_t xml_length = strlen(xml_data->ptr);

			// Used to check for the height tag
			char height_tag[] = "<height>";

			// Search through the xml
			size_t num_heights = 0;
			for (size_t i = 0; i < xml_length - 9; i++) {

				char* c = xml_data->ptr + i;
		
				// check for height tag
				bool is_height_tag = true;
				for (size_t j = 0; j < strlen(height_tag); j++) {
					if (c[j] != height_tag[j]) {
						is_height_tag = false;
						break;
					}
				}
	
				// Continue
				if (!is_height_tag)
					continue;

				// Get the number
				char* height_string = (char*)malloc(sizeof(char) * 20);
				for (size_t j = 0; j < 20; j++) {
					char char_to_add = xml_data->ptr[i+8+j];
					if (char_to_add == '<') {
						height_string[j] = '\0';
						break;
					}
					else {
						height_string[j] = char_to_add;
					}
				}
	
				float height = atof(height_string);
				std::cout << height << std::endl;

				heights[num_heights++] = height;

				free(height_string);
			}
			for (char* c = xml_data->ptr; c[0] != '\0'; c++) {
			} 
		}

		// Clean up libcurl
		curl_global_cleanup();
	}
	else {
		// Throw custom error
	}
}

ll_map::ll_map() {
}

ll_map::~ll_map() {
	clean_map();
}

void ll_map::build_map(double lat, double lon, double width_meters, size_t density) {
	// Make sure density is odd so there's a center point
	density += (density % 2 == 0) ? 1 : 0;

	// Setup the member variables
	m_latitude = lat;
	m_longitude = lon;
	m_width_meters = width_meters;
	m_width_degrees = width_meters / 111044.736;
	m_density = density;
	m_spacing_meters = m_width_meters / m_density;
	m_spacing_degrees = m_width_degrees / m_density;

	// Calculate the upper left for easy calculations later on
	m_upper_left_lat = m_latitude - (m_width_degrees / 2);
	m_upper_left_lon = m_longitude - (m_width_degrees / 2);
	
	// Setup the map
	m_map = new float*[m_density];
	for (int i=0; i<m_density; i++) {
		m_map[i] = new float[m_density];
	}

	// pull restful data
	size_t total_so_far = 0;
	size_t num_points = m_density * m_density;
	size_t num_points_to_read = 0;
	const size_t MAX_READ_POINTS = 400;
	while(true) {
		// Check if we're done
		if (total_so_far == num_points)
			break;

		// Figure out how many points we're going to read from this REST call.
		num_points_to_read = num_points - total_so_far;
		if (num_points_to_read > MAX_READ_POINTS)
			num_points_to_read = MAX_READ_POINTS;

		// Build arrays for temporary data
		double* ll;
		double* heights;
		heights = (double*)malloc(sizeof(double) * num_points_to_read);
		ll = (double*)malloc(sizeof(double) * num_points_to_read * 2);

		// Determine the latitudes and longitudes of the points
		for (size_t i=0; i<num_points_to_read*2; i+=2) {
			// Determine the index
			size_t x, y;
			from_int_to_xy(total_so_far + (i/2), &x, &y);
			
			// Determine the latitude and longitude
			double lat, lon;
			from_xy_to_ll(x, y, &lat, &lon);

			// Dump the values into 'll'
			ll[i] = lat;
			ll[i+1] = lon;

			//std::cout << "lat: " << lat << std::endl;
			//std::cout << "lon: " << lon << std::endl;
		}
	
		// Make the REST call
		pull_data(ll, heights, num_points_to_read);

		// Dump the data into 'm_map'
		for (size_t i=0; i<num_points_to_read*2; i+=2) {
			// Determine the index... again
			size_t x, y;
			from_int_to_xy(total_so_far + (i/2), &x, &y);

			// place data
			m_map[x][y] = heights[i/2];
		}
		
		// Increment 'total_so_far'
		total_so_far += num_points_to_read;

		// Clear memory from temporary data
		free(ll);
		free(heights);
	}
}

void ll_map::clean_map() {
	delete[] m_map;
}

void ll_map::from_int_to_xy(size_t index, size_t* x, size_t* y) {
	*y = floor((double)index/ (double)m_density);
	*x = index % m_density;
}

void ll_map::from_ll_to_xy(double lat, double lon, size_t* x, size_t* y) {
	double new_x, new_y;
	
	new_x = (lat - m_upper_left_lat) / m_spacing_degrees;
	new_y = (lon - m_upper_left_lon) / m_spacing_degrees;

	// Round the float to an int properly
	*x = floor(new_x + 0.5);
	*y = floor(new_y + 0.5);
}

void ll_map::from_xy_to_int(size_t x, size_t y, size_t* index) {
	*index = m_density * y + x;
}

void ll_map::from_xy_to_ll(size_t x, size_t y, double* lat, double* lon) {
	double new_lat, new_lon;
	
	new_lat = m_upper_left_lat + x * m_spacing_degrees;
	new_lon = m_upper_left_lon + y * m_spacing_degrees;
	
	// Set the new latitudes and longitudes
	*lat = new_lat;
	*lon = new_lon;
}

float ll_map::get_height(size_t x, size_t y) {
	if (x >= m_density || y >= m_density)
		return 0;

	return m_map[x][y];
}

}
