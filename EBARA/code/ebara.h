#ifndef ebara
#define ebara

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>
#include <algorithm>
#include <thread>
#include <numeric>
#include "utils.h"
#include <stdexcept>
#include <cstdlib>
#include "object.h"

//set_filename
void set_filename_by_parameters(std::vector<double>machine_parameters, Path& path);
//set filename 1.csv -> 000001.csv
std::string set_filename(int original_filename, int padding_size);
//locus of the dresser on pad
void LocusSimulation(Locus_coordinate& locus_coordinate, Diamonds_coordinate& diamonds_coordinate, CMP_Parameter machine_parameter);
void show_locus(Locus_coordinate& locus_coordinate, Image_setup image_setup, int pad_speed, const std::string& save_path);
cv::Mat color_map(const Array2D<double>& array, Image_setup image_setup, int max, int min, bool stats_flag);
cv::Mat color_map(const Array2D<int>& array, Image_setup image_setup, int max, int min, bool stats_flag);
std::vector<std::array<int, 2>>  get_line_pixels(double x1, double y1, double x2, double y2, float& m, float& c);
void get_line_coordinate(double x1, double y1, double x2, double y2, std::vector<std::vector<double>>(&line_coordinate)[SIZE][SIZE]);
void get_all_line_coordinate(Locus_coordinate locus_coordinate, Image_setup image_setup, std::vector<std::vector<double>>(&line_coordinate)[SIZE][SIZE]);
int count_elements(const std::vector<std::vector<double>> (&line_coordinate)[SIZE][SIZE]);
bool validate_conversion(const std::vector<std::vector<double>> (&line_coordinate)[SIZE][SIZE], const PixelLineSegments& pixelData);
PixelLineSegments convert_array4d_to_pixelData(const std::vector<std::vector<double>> (&line_coordinate)[SIZE][SIZE]);
void trajectory_distance(const int start, const int end, const PixelLineSegments& pixelData, PAP_distribution& mrr);
void multi_thread_trajectory_distance(const PixelLineSegments& pixelData, PAP_distribution& mrr_pixel_level);
bool check_overlap(double* line1, double* line2);
double calculate_overlap_angle(double* line1, double* line2);
void intersection_and_angle(const int start, const int end, const PixelLineSegments& pixelData, PAP_distribution& mrr_pixel_level);
void multi_thread_intersection_and_angle(const PixelLineSegments& pixelData, PAP_distribution& mrr_pixel_level);
void PAP_calculation(PAP_distribution& mrr_pixel_level);
PAP_distribution pixel2block_level(const PAP_distribution& mrr_pixel_level, const Image_setup& image_info);

void whole_wafer_locus_simulation(Locus_coordinate &whole_wafer_coordinate, Diamonds_coordinate &wafer_points, CMP_Parameter machine_parameter, Wafer_spec wafer_spec);
PAP_distribution whole_wafer_mrr_calculation(Locus_coordinate &wafer_coordinate, Image_setup &image_info, PAP_distribution &pap_block_level, Diamonds_coordinate &wafer_points, Wafer_spec wafer_spec);

#endif // !ebara