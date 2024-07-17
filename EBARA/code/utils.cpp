#define  _USE_MATH_DEFINES
#include <cmath>
#include <opencv2/opencv.hpp>
#include <dirent.h> // for directory operations
#include <cstring>  // for strcmp
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "utils.h"

namespace fs = std::filesystem;

/**
 * Creates a file at the specified path.
 * 
 * @param path The path of the file to be created.
 */
void create_file(const std::string& path) {
    // use ofstream to create a file or check if it exists
    std::ofstream ofs(path);
    if (!ofs) {
        std::cerr << "Error: Could not create file at " << path << std::endl;
    }
}

/**
 * Creates a directory at the specified path.
 * 
 * @param path The path of the directory to be created.
 */
void create_dir(const std::string& path) {
    try {
        if (fs::create_directories(path)) {
            std::cout << "Directory created successfully: " << path << std::endl;
        } else {
            std::cout << "Directory already exists or failed to create: " << path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "General error: " << e.what() << std::endl;
    }
}

/**
 * Retrieves the names of all files in a directory.
 * 
 * @param path The path to the directory.
 * @param files A vector to store the names of the files.
 */
void getFiles(const std::string& path, std::vector<std::string>& files) {
    DIR* dir;
    struct dirent* ent;

    // open directory
    if ((dir = opendir(path.c_str())) != NULL) {
        // read all files in the directory
        while ((ent = readdir(dir)) != NULL) {
            // ignore . and ..
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                files.push_back(ent->d_name);
            }
        }
        // close directory
        closedir(dir);
    } else {
        // if directory does not exist
        std::cerr << "Error: Could not open directory at " << path << std::endl;
    }
}

/**
 * Converts swing speeds from millimeters to degrees.
 * 
 * @param swing_speed The vector of swing speeds in millimeters.
 * @param ra The radius of the swing in millimeters.
 * @return The vector of swing speeds converted to degrees.
 */
std::vector<double> mm2deg(std::vector<double> swing_speed, double ra)
{
	for (int i = 0; i < 11; i++)
	{
		std::cout << swing_speed[i] << " ";
		swing_speed[i] = swing_speed[i] * 180 / ra / M_PI;
	}
	return swing_speed;
}

void cumsum(std::vector<int> input, std::vector<int>& result)
{
	int sum = 4;
	result.push_back(sum);
	for (int i = 0; i < input.size(); i++)
	{
		sum += input[i];
		result.push_back(sum);
	}
}

double deg2rad(double input)
{
	input = input * M_PI / 180;
	return input;
}
std::vector<double> deg2rad(std::vector<double> input)
{
	for (int i = 0; i < input.size(); i++)
	{
		input[i] = input[i] * M_PI / 180;
	}
	return input;
}

double rad2deg(double input)
{
	return input * (180.0 / M_PI);
}

double rpm2rad(double input)
{
	return input = 2 * M_PI * input / 60;
}

std::vector<std::vector<double>> readCSV(const std::string& filename)
{
	std::vector<std::vector<double>> data;
	std::ifstream file(filename);

	if (!file) {
		std::cerr << "Failed to open the file: " << filename << std::endl;
		return data;
	}

	std::string line;
	getline(file, line);
	while (getline(file, line)) {
		std::vector<double> row;
		std::stringstream ss(line);
		std::string cell;

		while (getline(ss, cell, ',')) {
			double value = stod(cell);
			row.push_back(value);
		}

		data.push_back(row);
	}

	return data;
}

void saveArray2DToCSV(const Array2D<double>& array, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    int dim1, dim2;
    array.get_dimension(dim1, dim2);

    for (int i = 0; i < dim1; ++i) {
        for (int j = 0; j < dim2; ++j) {
            file << array.get_data(i, j);
            if (j < dim2 - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

void saveArray2DToCSV(const Array2D<int>& array, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    int dim1, dim2;
    array.get_dimension(dim1, dim2);

    for (int i = 0; i < dim1; ++i) {
        for (int j = 0; j < dim2; ++j) {
            file << array.get_data(i, j);
            if (j < dim2 - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

double avg(std::vector<int> input)
{
	if (input.empty())
		return 0;
	long sum = 0;
	for (int i = 0; i < input.size(); i++)
	{
		sum += input[i];
	}

	return sum / float(input.size());
}


template <typename T>
void min_max_norm(T input[375][375], double output[375][375], int size)
{
	double min = DBL_MAX, max = DBL_MIN;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (input[i][j] > max) max = input[i][j];
			if (input[i][j]< min) min = input[i][j];
		}
	}
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			output[i][j] = (input[i][j] - min) / (max - min);
		}
	}

}
template void min_max_norm<int>(int input[375][375], double output[375][375], int size);
template void min_max_norm<double>(double input[375][375], double output[375][375], int size);


float distance(float x1, float y1, float x2, float y2) {
	return sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
}

void resize_array(double input[375][375], double output[375][375], int picture_size, int origin_size, int times) {

	int new_size = origin_size * times;
	if (new_size > picture_size) {

		std::cout << "ERROR!!!! Resize to much times\n.";

	} // end if

	for (int i = 0; i < picture_size; i++) {

		for (int j = 0; j < picture_size; j++) {

			output[i][j] = input[i / times][j / times];

		} // end for 

	} // end for

}
void resize_array(int input[375][375], double output[375][375], int picture_size, int origin_size, int times) {

	int new_size = origin_size * times;
	if (new_size > picture_size) {

		std::cout << "ERROR!!!! Resize too much times\n.";

	} // end if

	for (int i = 0; i < picture_size; i++) {

		for (int j = 0; j < picture_size; j++) {

			output[i][j] = input[i / times][j / times];

		} // end for 

	} // end for

}
void clipLineSegment(double& x1, double& y1, double& x2, double& y2, 
                     double minX, double minY, double maxX, double maxY) {
    // 計算斜率
    double dx = x2 - x1;
    double dy = y2 - y1;
    
    // 定義一個 lambda 函數來處理單個點的截斷
    auto clipPoint = [&](double& x, double& y) {
        // 處理 x < minX 的情況
        if (x < minX) {
            double t = (minX - x) / dx;
            x = minX;
            y = y + t * dy;
        }
        // 處理 x > maxX 的情況
        else if (x > maxX) {
            double t = (maxX - x) / dx;
            x = maxX;
            y = y + t * dy;
        }
        
        // 處理 y < minY 的情況
        if (y < minY) {
            double t = (minY - y) / dy;
            y = minY;
            x = x + t * dx;
        }
        // 處理 y > maxY 的情況
        else if (y > maxY) {
            double t = (maxY - y) / dy;
            y = maxY;
            x = x + t * dx;
        }
    };
    
    // 對兩個端點應用截斷
    clipPoint(x1, y1);
    clipPoint(x2, y2);
}