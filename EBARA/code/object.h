#ifndef OBJECT_H
#define OBJECT_H

#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

const int SIZE = 250;

struct Path
{
	std::string save_path = "";
	std::string diamond_path = "";
	std::string save_name = "";
	std::string wafer_path = "";
	std::string PAP_path = "";
	std::string MRR_path = "";

};

class Locus_coordinate
{
public:
	std::vector<std::vector<double>> x;
	std::vector<std::vector<double>> y;
	void clearall() {
		x.clear();
		y.clear();
	}
};

struct Wafer_spec
{
	int size; //wafer_size
	int location; //wafer_to_pad_center_distance
	int rotation_speed;
	double distance_from_center;//distance_from_wafer_center
};

class Image_setup
{
public:
	int img_size; // pad size
	int block_size; //how many pixels for block. For example, 2x2 block_size=2
	int diamond_size; 
	int save_size; // save_size=img_size/block_size
};

class Diamonds_coordinate
{
public:
	std::vector<double> x, y, theta, rho;
	void Load_Diamond_Coordinates_Swingarm(std::string path);
	void Cart2Pol();
};

struct CMP_Parameter
{
	double np, nd, vs, cycle;
	void print_parameter();

};

class PixelLineSegments {
private:
    double* data;
    int* data_index;
    int* line_num;
    int total_size, pixel_size;

    int index(int i, int j, int k, int l) const;

public:
    PixelLineSegments();
    PixelLineSegments(int total_size, int pixel_size);
    ~PixelLineSegments();

    double get_data(int i, int j, int k, int l) const;
    void set_data(int i, int j, int k, int l, double value);
    int get_pixel_line_num(int i, int j) const;
    void set_pixel_line_num(int i, int j, int value);
    void set_pixel_start_index(int i, int j, int value);
};

template<typename T>
class Array2D {
private:
    T* data;
    int dim1, dim2;

    int index(int i, int j) const;

public:
    Array2D();
    Array2D(int dim1, int dim2);
    ~Array2D();

    // Copy constructor
    Array2D(const Array2D& other);
    // Move constructor
    Array2D(Array2D&& other) noexcept = default;

    // Copy assignment operator
    Array2D& operator=(const Array2D& other);
    // Move assignment operator
    Array2D& operator=(Array2D&& other) noexcept = default;

    T get_data(int i, int j) const;
    void set_data(int i, int j, T value);
    void get_dimension(int& dim1, int& dim2) const;
};

class PAP_distribution {
private:
    int SIZE;
    Array2D<int> intersection;
    Array2D<double> angle;
    Array2D<double> distance;
    Array2D<double> mrr;

public:
    PAP_distribution(int SIZE);
    ~PAP_distribution() = default;

    int get_size() const;
    void set_intersection(int i, int j, int value);
    int get_intersection(int i, int j) const;
    Array2D<int> get_intersection_array() const;
    void set_angle(int i, int j, double value);
    double get_angle(int i, int j) const;
    Array2D<double> get_angle_array() const;
    void set_distance(int i, int j, double value);
    double get_distance(int i, int j) const;
    Array2D<double> get_distance_array() const;
    void set_mrr(int i, int j, double value);
    double get_mrr(int i, int j) const;
    Array2D<double> get_mrr_array() const;
};

#endif // OBJECT_H