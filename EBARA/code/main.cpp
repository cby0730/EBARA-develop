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

void run_single_parameter_set_mode2(const std::vector<double>& param)
{
	Locus_coordinate locus_coordinate, wafer_coordiante;
	std::vector<std::vector<double>> line_coordinate[SIZE][SIZE];
	cv::Mat result;

	// set up parameters
	CMP_Parameter machine_parameters{ param[0], param[1] ,param[2], param[3] };
	// machine_parameters.print_parameter();

	// use the parameters to generate the file name. 
	// For example: 60_60_10_2
	set_filename_by_parameters(param, File_Path);

	// generate locus of the dresser on pad
	LocusSimulation(locus_coordinate, diamonds_coordinate, machine_parameters);
	// show_locus(locus_coordinate, image_info, machine_parameters.np, (File_Path.save_path + "locus_" + File_Path.save_name));

	auto begin = std::chrono::high_resolution_clock::now();

	//line_coordinate
	get_all_line_coordinate(locus_coordinate, image_info, line_coordinate);
	
	PixelLineSegments pixelData = convert_array4d_to_pixelData(line_coordinate); // convert vector to pointer array

	// ================================================pap calculation================================================
	PAP_distribution pap_pixel_level(SIZE);

	std::thread t1(multi_thread_trajectory_distance, std::ref(pixelData), std::ref(pap_pixel_level));
	std::thread t2(multi_thread_intersection_and_angle, std::ref(pixelData), std::ref(pap_pixel_level));
	t1.join();
	t2.join();

	PAP_calculation(pap_pixel_level);
	PAP_distribution pap_block_level = pixel2block_level(pap_pixel_level, image_info);
	
	// save the result to csv and image
	saveArray2DToCSV(pap_block_level.get_mrr_array(), File_Path.save_path + "PAP_" + File_Path.save_name + ".csv");
	result = color_map(pap_block_level.get_mrr_array(), image_info, 0, 0, true);
	cv::imwrite(File_Path.save_path + "PAP_" + File_Path.save_name + ".png", result);

	// ================================================whole wafer================================================
	whole_wafer_locus_simulation(wafer_coordiante, wafer_points, machine_parameters, wafer_info);
	PAP_distribution mrr_block_level = whole_wafer_mrr_calculation(wafer_coordiante, image_info, pap_block_level, wafer_points, wafer_info);
	
	// save the result to csv and image
	saveArray2DToCSV(mrr_block_level.get_mrr_array(), File_Path.save_path + "MRR_" + File_Path.save_name + ".csv");
	result = color_map(mrr_block_level.get_mrr_array(), image_info, 0, 0, true);
	cv::imwrite(File_Path.save_path + "MRR_" + File_Path.save_name + ".png", result);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

	// std::cout << "Time taken by the whole process: " << elapsed.count() << " milliseconds" << std::endl;
}

void run_single_parameter_set_mode3(const std::vector<double>& param)
{
	Locus_coordinate locus_coordinate, wafer_coordiante;
	std::vector<std::vector<double>> line_coordinate[SIZE][SIZE];
	cv::Mat result;
	// fill 0 to filename. For example 000001.csv or 000001.jpg
	std::string filename = set_filename(param[0], 6);

	// set up parameters
	CMP_Parameter machine_parameters{ param[1], param[2] ,param[3], param[4] };
	// machine_parameters.print_parameter();

	// generate locus of the dresser on pad
	LocusSimulation(locus_coordinate, diamonds_coordinate, machine_parameters);
	// show_locus(locus_coordinate, image_info, machine_parameters.np, (File_Path.save_path + "locus_" + File_Path.save_name));

	auto begin = std::chrono::high_resolution_clock::now();

	//line_coordinate
	get_all_line_coordinate(locus_coordinate, image_info, line_coordinate);
	
	PixelLineSegments pixelData = convert_array4d_to_pixelData(line_coordinate); // convert vector to pointer array

	// ================================================pap calculation================================================
	PAP_distribution pap_pixel_level(SIZE);

	std::thread t1(multi_thread_trajectory_distance, std::ref(pixelData), std::ref(pap_pixel_level));
	std::thread t2(multi_thread_intersection_and_angle, std::ref(pixelData), std::ref(pap_pixel_level));
	t1.join();
	t2.join();

	PAP_calculation(pap_pixel_level);
	PAP_distribution pap_block_level = pixel2block_level(pap_pixel_level, image_info);
	
	// save the result to csv and image
	saveArray2DToCSV(pap_block_level.get_mrr_array(), File_Path.PAP_path + filename + ".csv");
	result = color_map(pap_block_level.get_mrr_array(), image_info, 0, 0, true);
	cv::imwrite(File_Path.PAP_path + filename + ".png", result);

	// ================================================whole wafer================================================
	whole_wafer_locus_simulation(wafer_coordiante, wafer_points, machine_parameters, wafer_info);
	PAP_distribution mrr_block_level = whole_wafer_mrr_calculation(wafer_coordiante, image_info, pap_block_level, wafer_points, wafer_info);
	
	// save the result to csv and image
	saveArray2DToCSV(mrr_block_level.get_mrr_array(), File_Path.MRR_path + filename + ".csv");
	result = color_map(mrr_block_level.get_mrr_array(), image_info, 0, 0, true);
	cv::imwrite(File_Path.MRR_path + filename + ".png", result);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

	// std::cout << "Time taken by the whole process: " << elapsed.count() << " milliseconds" << std::endl;
}

void run(int mode, std::string csv_file_name)
{
	int MRR_mode = 2, num_processes = 20;
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
			{30, 30, 10, 2},
			//{20,0,10,2},{20,10,10,2},{20,20,10,2},{20,30,10,2},{20,40,10,2},{20,50,10,2},{20,60,10,2},{20,70,10,2},{20,80,10,2},{20,90,10,2},{20,100,10,2},
			//{30,0,10,2},{30,10,10,2},{30,20,10,2},{30,30,10,2},{30,40,10,2},{30,50,10,2},{30,60,10,2},{30,70,10,2},{30,80,10,2},{30,90,10,2},{30,100,10,2},
			//{40,0,10,2},{40,10,10,2},{40,20,10,2},{40,30,10,2},{40,40,10,2},{40,50,10,2},{40,60,10,2},{40,70,10,2},{40,80,10,2},{40,90,10,2},{40,100,10,2},
			//{50,0,10,2},{50,10,10,2},{50,20,10,2},{50,30,10,2},{50,40,10,2},{50,50,10,2},{50,60,10,2},{50,70,10,2},{50,80,10,2},{50,90,10,2},{50,100,10,2},
			//{60,0,10,2},{60,10,10,2},{60,20,10,2},{60,30,10,2},{60,40,10,2},{60,50,10,2},{60,60,10,2},{60,70,10,2},{60,80,10,2},{60,90,10,2},{60,100,10,2},
			//{70,0,10,2},{70,10,10,2},{70,20,10,2},{70,30,10,2},{70,40,10,2},{70,50,10,2},{70,60,10,2},{70,70,10,2},{70,80,10,2},{70,90,10,2},{70,100,10,2},
			//{80,0,10,2},{80,10,10,2},{80,20,10,2},{80,30,10,2},{80,40,10,2},{80,50,10,2},{80,60,10,2},{80,70,10,2},{80,80,10,2},{80,90,10,2},{80,100,10,2},
			//{90,0,10,2},{90,10,10,2},{90,20,10,2},{90,30,10,2},{90,40,10,2},{90,50,10,2},{90,60,10,2},{90,70,10,2},{90,80,10,2},{90,90,10,2},{90,100,10,2},
			//{100,0,10,2},{100,10,10,2},{100,20,10,2},{100,30,10,2},{100,40,10,2},{100,50,10,2},{100,60,10,2},{100,70,10,2},{100,80,10,2},{100,90,10,2},{100,100,10,2},
		};

		int total_tasks = cmp_parameters.size();
		int completed_tasks = 0;

		for (size_t i = 0; i < cmp_parameters.size(); i += num_processes) {
			std::vector<pid_t> child_pids;

			for (int j = 0; j < num_processes && i + j < cmp_parameters.size(); ++j) {
				pid_t pid = fork();

				if (pid == 0) {  // Child process
					run_single_parameter_set_mode2(cmp_parameters[i + j]);
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
		std::vector<std::string> csv_files;
		getFiles("../param_split/", csv_files);

		for (auto &csv_file : csv_files)
		{
			// execute only the specified csv file
			if (csv_file != csv_file_name)
				continue;

			// train.csv -> save_dir/train/
			File_Path.save_path = File_Path.save_path + csv_file.substr(0, csv_file.length() - 4);
			create_dir(File_Path.save_path);
			// create save_dir/train/PAP_distribution
			File_Path.PAP_path = File_Path.save_path + "/PAP_distribution/";
			create_dir(File_Path.PAP_path);
			// create save_dir/train/Wafer_distribution
			File_Path.MRR_path = File_Path.save_path + "/MRR_distribution/";
			create_dir(File_Path.MRR_path);

			std::vector<std::vector<double>> machine_parameters;
			machine_parameters = readCSV("../param_split/" + csv_file);

			int total_tasks = machine_parameters.size();
			int completed_tasks = 0;

			for (size_t i = 0; i < machine_parameters.size(); i += num_processes) {
				std::vector<pid_t> child_pids;

				for (int j = 0; j < num_processes && i + j < machine_parameters.size(); ++j) {
					pid_t pid = fork();

					if (pid == 0) {  // Child process
						run_single_parameter_set_mode3(machine_parameters[i + j]);
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
	if (wafer_info.size == 50) {
		File_Path.wafer_path = "../wafer_points/circle.csv";
	} else if (wafer_info.size == 15) {
		File_Path.wafer_path = "../wafer_points/square.csv";
	} else {
		std::cerr << "wafer size error" << std::endl;
		File_Path.wafer_path = "";
		throw std::runtime_error("Invalid wafer size");
	}
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
	int mode = 3; 
	std::string csv_file_name = "test_time.csv";
	run(mode, csv_file_name);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	std::cout << "Time taken by the whole process: " << elapsed.count() << " milliseconds" << std::endl;

	return 0;

}
