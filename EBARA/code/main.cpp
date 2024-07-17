#include "ebara.h"
#include "utils.h"
#include "object.h"
#include <array>
#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>


Diamonds_coordinate diamonds_coordinate, wafer_points;
Path File_Path;

Wafer_spec wafer_info;
Image_setup image_info;

void saveCSV_and_img(const PAP_distribution& mrr_block_level)
{
	//std::cout << "1. Store data into csv" << std::endl;
	saveArray2DToCSV(mrr_block_level.get_intersection_array(), File_Path.save_path + "intersection_" + File_Path.save_name + ".csv");
	saveArray2DToCSV(mrr_block_level.get_angle_array(), File_Path.save_path + "angle_" + File_Path.save_name + ".csv");
	saveArray2DToCSV(mrr_block_level.get_distance_array(), File_Path.save_path + "distance_" + File_Path.save_name + ".csv");
	saveArray2DToCSV(mrr_block_level.get_mrr_array(), File_Path.save_path + "mrr_" + File_Path.save_name + ".csv");

	cv::Mat result;

	//std::cout << "2. Save image" << std::endl;
	result = color_map(mrr_block_level.get_mrr_array(), image_info, 0, 0, true);
	cv::imwrite(File_Path.save_path + "mrr_" + File_Path.save_name + ".png", result);
	result = color_map(mrr_block_level.get_distance_array(), image_info, 0, 0, true);
	cv::imwrite(File_Path.save_path + "distance_" + File_Path.save_name + ".png", result);
	result = color_map(mrr_block_level.get_angle_array(), image_info, 0, 0, true);
	cv::imwrite(File_Path.save_path + "angle_" + File_Path.save_name + ".png", result);
	result = color_map(mrr_block_level.get_intersection_array(), image_info, 0, 0, true);
	cv::imwrite(File_Path.save_path + "intersection_" + File_Path.save_name + ".png", result);
}

void run_single_parameter_set(const std::vector<double>& param)
{
	Locus_coordinate locus_coordinate, wafer_coordiante;
	std::vector<std::vector<double>> line_coordinate[SIZE][SIZE];

	// set up parameters
	CMP_Parameter machine_parameters{ param[0], param[1] ,param[2], param[3] };
	// machine_parameters.print_parameter();

	// use the parameters to generate the file name. 
	// For example: 60_60_10_2
	set_filename_by_parameters(param, File_Path);

	// generate locus of the dresser on pad
	//std::cout << "\nLocusSimulation" << std::endl;
	LocusSimulation(locus_coordinate, diamonds_coordinate, machine_parameters);
	// show_locus(locus_coordinate, image_info, machine_parameters.np, (File_Path.save_path + "locus_" + File_Path.save_name));

	auto begin = std::chrono::high_resolution_clock::now();

	//line_coordinate
	//std::cout << "\nGet all line coordinate" << std::endl;
	get_all_line_coordinate(locus_coordinate, image_info, line_coordinate);
	
	//std::cout << "\nConvert vector to pointer array" << std::endl;
	PixelLineSegments pixelData = convert_array4d_to_pixelData(line_coordinate); // convert vector to pointer array

	PAP_distribution mrr_pixel_level(SIZE);

	//std::cout << "\nTrajectory distance" << std::endl;
	std::thread t1 (multi_thread_trajectory_distance, std::ref(pixelData), std::ref(mrr_pixel_level));

	//std::cout << "\nIntersection and angle" << std::endl;
	std::thread t2(multi_thread_intersection_and_angle, std::ref(pixelData), std::ref(mrr_pixel_level));

	t1.join();
	t2.join();

	//std::cout << "\nPAP calculation" << std::endl;
	PAP_calculation(mrr_pixel_level);
	
	//std::cout << "\nPixel to block level" << std::endl;
	PAP_distribution mrr_block_level = pixel2block_level(mrr_pixel_level, image_info);

	//std::cout << "\nStore data into csv and image: " << std::endl;
	saveCSV_and_img(mrr_block_level);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

	//std::cout << "Time taken by the whole process: " << elapsed.count() << " milliseconds" << std::endl;
}

void run(int mode, std::string csv_file_name)
{
	int MRR_mode = 2, num_processes = 60;
	if (mode == 0)
	{
		// test new function or certain parameter
	}
	else if (mode == 1)
	{
		// generate data for PAP distribution
	}
	else if (mode == 2)
	{
		std::vector<std::string> file_names;
		std::cout << "Run mode 2: for whole wafer" << std::endl;
		std::vector<std::vector<double>> cmp_parameters{  //ps=pad speed,ds=dresser speed,dms=dresser moving speed,cycle
			//{30, 60, 10, 2},
			{20,0,10,2},{20,10,10,2},{20,20,10,2},{20,30,10,2},{20,40,10,2},{20,50,10,2},{20,60,10,2},{20,70,10,2},{20,80,10,2},{20,90,10,2},{20,100,10,2},
			{30,0,10,2},{30,10,10,2},{30,20,10,2},{30,30,10,2},{30,40,10,2},{30,50,10,2},{30,60,10,2},{30,70,10,2},{30,80,10,2},{30,90,10,2},{30,100,10,2},
			{40,0,10,2},{40,10,10,2},{40,20,10,2},{40,30,10,2},{40,40,10,2},{40,50,10,2},{40,60,10,2},{40,70,10,2},{40,80,10,2},{40,90,10,2},{40,100,10,2},
			{50,0,10,2},{50,10,10,2},{50,20,10,2},{50,30,10,2},{50,40,10,2},{50,50,10,2},{50,60,10,2},{50,70,10,2},{50,80,10,2},{50,90,10,2},{50,100,10,2},
			{60,0,10,2},{60,10,10,2},{60,20,10,2},{60,30,10,2},{60,40,10,2},{60,50,10,2},{60,60,10,2},{60,70,10,2},{60,80,10,2},{60,90,10,2},{60,100,10,2},
			{70,0,10,2},{70,10,10,2},{70,20,10,2},{70,30,10,2},{70,40,10,2},{70,50,10,2},{70,60,10,2},{70,70,10,2},{70,80,10,2},{70,90,10,2},{70,100,10,2},
			{80,0,10,2},{80,10,10,2},{80,20,10,2},{80,30,10,2},{80,40,10,2},{80,50,10,2},{80,60,10,2},{80,70,10,2},{80,80,10,2},{80,90,10,2},{80,100,10,2},
			{90,0,10,2},{90,10,10,2},{90,20,10,2},{90,30,10,2},{90,40,10,2},{90,50,10,2},{90,60,10,2},{90,70,10,2},{90,80,10,2},{90,90,10,2},{90,100,10,2},
			{100,0,10,2},{100,10,10,2},{100,20,10,2},{100,30,10,2},{100,40,10,2},{100,50,10,2},{100,60,10,2},{100,70,10,2},{100,80,10,2},{100,90,10,2},{100,100,10,2},
		};

		int total_tasks = cmp_parameters.size();
		int completed_tasks = 0;

		for (size_t i = 0; i < cmp_parameters.size(); i += num_processes) {
			std::vector<pid_t> child_pids;

			for (int j = 0; j < num_processes && i + j < cmp_parameters.size(); ++j) {
				pid_t pid = fork();

				if (pid == 0) {  // Child process
					run_single_parameter_set(cmp_parameters[i + j]);
					exit(0);
				} else if (pid < 0) {  // Fork failed
					std::cerr << "Fork failed" << std::endl;
					exit(1);
				} else {  // Parent process
					child_pids.push_back(pid);
				}
			}

			// Wait for all child processes in this batch to complete
			for (pid_t pid : child_pids) {
				int status;
				waitpid(pid, &status, 0);
				completed_tasks++;
			}

			// Print progress
			std::cout << "Completed " << completed_tasks << " out of " 
				<< total_tasks << " tasks\r" << std::flush;
		}

		std::cout << std::endl << "All tasks completed." << std::endl;

	}
	else if (mode == 3)
	{
		// generate data for MRR distribution
	}
	else
	{
		printf("mode error\n");
	}

} // if you see mrr corrupted at here, you can take a look at wafer_spec.size. Perhaps you forget to change the value.


int main()
{
	// set up pad and image information
	image_info.img_size = SIZE;
	image_info.block_size = 2;
	image_info.save_size = image_info.img_size / image_info.block_size;
	image_info.diamond_size = 504;

	// set up wafer information
	wafer_info.size = 50;
	wafer_info.location = 55;
	wafer_info.rotation_speed = 100;
	wafer_info.distance_from_center = 0;

	// set up file path
	File_Path.diamond_path = "../diamond/" + std::to_string(image_info.diamond_size) + ".csv";
	File_Path.save_path = "../save/" + std::to_string(image_info.img_size) + "mm_D" + std::to_string(image_info.diamond_size) + "_mathamatical/";
	File_Path.wafer_path = "../wafer_points/circle.csv";
	create_dir(File_Path.save_path);

	if (diamonds_coordinate.x.empty() | diamonds_coordinate.y.empty())
	{
		diamonds_coordinate.Load_Diamond_Coordinates_Swingarm(File_Path.diamond_path);
		diamonds_coordinate.Cart2Pol();
	}

	if (wafer_points.x.empty() | wafer_points.y.empty())
	{
		wafer_points.Load_Diamond_Coordinates_Swingarm(File_Path.wafer_path);
		wafer_points.Cart2Pol();
	}

	auto begin = std::chrono::high_resolution_clock::now();
	
	// 0 for test new function or certain parameter
	// 1 for generate data for PAP distribution
	// 2 for test wafer MRR distribution
	// 3 for generate data for MRR distribution
	int mode = 2; 
	std::string csv_file_name = "test_time.csv";
	run(mode, csv_file_name);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	std::cout << "Time taken by the whole process: " << elapsed.count() << " milliseconds" << std::endl;

	return 0;

}
