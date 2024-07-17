#ifndef utils
#define utils

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>
#include "ebara.h"
#include "object.h"


struct Flag
{
	bool locus_flag;
	bool threeD_flag;
	bool error_flag;
	bool counterclockwise;
	bool csv_flag;
	bool color_bar;
	bool intersection; //y1
	bool cutting_angle; //y2
	bool trajectory_distance; //y3
	bool synchronize_locus_intersection_angle;

};

void create_file(const std::string& path);
void create_dir(const std::string& path);
void getFiles(const std::string& path, std::vector<std::string>& files);
void cumsum(std::vector<int> input, std::vector<int>& result);
//  mm to deg
std::vector<double> mm2deg(std::vector<double> swing_speed, double ra); // ra���\�u
//  deg to rad
double deg2rad(double input);
std::vector<double> deg2rad(std::vector<double> input);
//  rad to deg
double rad2deg(double input);
// rpm to rad/s
double rpm2rad(double input);

std::vector<std::vector<double>> readCSV(const std::string& filename);

void saveArray2DToCSV(const Array2D<double>& array, const std::string& filename);
void saveArray2DToCSV(const Array2D<int>& array, const std::string& filename);

//�p�⥭��
double avg(std::vector<int> input);
//Normalize 2D array
template <typename T>
void min_max_norm(T input[375][375], double output[375][375], int size);

void resize_array(double input[375][375], double output[375][375], int picture_size, int origin_size, int times);
void resize_array(int input[375][375], double output[375][375], int picture_size, int origin_size, int times);

float distance(float x1, float y1, float x2, float y2);
void clipLineSegment(double& x1, double& y1, double& x2, double& y2, double minX, double minY, double maxX, double maxY);


#endif // !utils
