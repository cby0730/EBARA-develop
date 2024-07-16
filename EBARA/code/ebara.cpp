#include "ebara.h"
#include <thread>
#include <cmath>
#include <math.h>
#include <iostream>
#include <limits>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

//set_filename
void set_filename_by_parameters(std::vector<double>machine_parameters, Path& path)
{
	std::stringstream int2string;
	for (int i = 0; i < machine_parameters.size(); i++)
	{
		int2string << (machine_parameters[i]) << "_";
	}

	path.save_name = int2string.str();
}
//set filename 1.csv -> 000001.csv
std::string set_filename(int original_filename, int padding_size) {

	std::string filename = std::to_string(original_filename);
	int precision = padding_size - min(padding_size, filename.size());
	filename.insert(0, precision, '0');
	return filename;
}
//locus
void LocusSimulation(Locus_coordinate& locus_coordinate, Diamonds_coordinate& diamonds_coordinate, CMP_Parameter machine_parameter)
{
	double wp, wd, vs, cycle, E, D, dressing_time, cycle_time, r, R, e, sample_rate = 100, t_temp;
	int ttime, tmp_ttime = 0, theta_size;

	// �]�w�p�ɾ�
	auto start_time = std::chrono::high_resolution_clock::now();

	vs = machine_parameter.vs;
	cycle = machine_parameter.cycle;
	e = 0;
	E = 25;//dresser���߲��ʶZ��
	D = 50;//dresser���߰_�l��m D+E���̻��Z��
	cycle_time = E * 2 / vs;
	dressing_time = cycle * cycle_time;

	std::vector<double> rho, theta;
	rho = diamonds_coordinate.rho;
	theta = diamonds_coordinate.theta;
	wp = rpm2rad(machine_parameter.np);
	wd = rpm2rad(machine_parameter.nd);
	std::vector<double>tmpx(cycle_time * 100 * cycle), tmpy(cycle_time * 100 * cycle);

	// FIX: add reserve for std::vector
	//tmpx.reserve(cycle_time * 100 * cycle);
	//tmpy.reserve(cycle_time * 100 * cycle);
	locus_coordinate.x.reserve(diamonds_coordinate.x.size());
	locus_coordinate.y.reserve(diamonds_coordinate.x.size());

	for (int i = 0; i < diamonds_coordinate.x.size(); i++)
	{
		r = sqrt(pow(rho[i] * cos(theta[i]), 2) + pow(rho[i] * sin(theta[i]), 2)); //diamond��dresser���ߪ�����
		tmpx.clear();
		tmpy.clear();
		e = 0;
		t_temp = 0;
		for (int j = 0; j < cycle; j++)
		{

			for (double t = t_temp; t < cycle_time * (j + 1); t = t + 1 / sample_rate)
			{
				R = D + e; //R:�ثedresser������pad���ߪ��Z��   r:diamond��dresser���ߪ��Z��
				tmpx.push_back(R * cos(-wp * t) + r * cos(theta[i] - wd * t));
				tmpy.push_back(R * sin(-wp * t) + r * sin(theta[i] - wd * t));
				if (t < cycle_time * j + cycle_time / 2)
					e += vs / sample_rate;
				else
					e -= vs / sample_rate;
			}
			t_temp = cycle_time * (j + 1);
		}
		locus_coordinate.x.push_back(tmpx);
		locus_coordinate.y.push_back(tmpy);
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

	//std::cout << "Time taken by the LocusSimulation: " << duration.count() << " milliseconds" << std::endl;

}
void show_locus(Locus_coordinate& locus_coordinate, Image_setup image_setup, int pad_speed, const std::string& save_path)
{
	int center = image_setup.img_size / 2;
	//center point
	double row = image_setup.img_size / 2;
	double col = image_setup.img_size / 2;
	// picture size 720*720
	cv::Mat src = cv::Mat::zeros(cv::Size(center * 2, center * 2), CV_8UC3); //locus�ϥ�
	src.setTo(255);

	cv::RNG rng((unsigned)time(NULL));
	std::vector<cv::Scalar> LineColor;
	LineColor.assign(locus_coordinate.x.size(), 0);
	for (size_t i = 0; i < locus_coordinate.x.size(); ++i)
	{
		LineColor[i] = cv::Scalar(rng.uniform(50, 250), rng.uniform(50, 250), rng.uniform(50, 250));
	}

	// �Ыض�α���
	cv::Mat mask = cv::Mat::zeros(cv::Size(center * 2, center * 2), src.type());
	cv::circle(mask, cv::Point(center, center), center*1.005, cv::Scalar(255, 255, 255), -1);

	cv::VideoWriter video(save_path + ".avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 60, cv::Size(center * 2, center * 2));
	if (!video.isOpened()) {
		std::cerr << "Error: Could not open the video file for write.In";
		return;
	}

	double angle_per_frame = (float(pad_speed) / 60) * 360 * 0.01;

	for (int j = 0; j < locus_coordinate.x[0].size() - 1; ++j)
	{
		for (int i = 0; i < locus_coordinate.x.size(); ++i)
		{
			//�̭쥻locus �|�@������e�짹
			line(src, cv::Point(locus_coordinate.x[i][j] + row, -locus_coordinate.y[i][j] + col), cv::Point(locus_coordinate.x[i][j + 1] + row,
				-locus_coordinate.y[i][j + 1] + col), LineColor[i], 1, cv::LINE_AA);
		}

		// �N�������Ψ�Ϲ��W
		src.setTo(cv::Scalar(0, 0, 0), mask == 0);

		double angle = j * angle_per_frame; // �ھڴV�ƭp����ਤ��
		cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point(center, center), angle, 1.0);

		// ����Ϲ�
		cv::Mat rotated;
		cv::warpAffine(src, rotated, rot_mat, src.size());

		video.write(rotated);
		//cv::waitKey(1);
		//cv::imshow("src", src);
	}
	
	video.release();

	// convert avi to mp4
    const std::string command = "ffmpeg -i " + save_path + ".avi " + save_path + ".mp4 -y";
	// use system to call ffmpeg command
    int result = std::system(command.c_str());
    
    // check if the command is executed successfully
    if (result == 0) {
        std::cout << "Convert successfully" << std::endl;
    } else {
        std::cerr << "Convert failed, failed code: " << result << std::endl;
    }

}
cv::Mat color_map(const Array2D<double>& array, Image_setup image_setup, int max, int min, bool stats_flag)
{
	double sum = 0.0, mean, variance = 0.0, std_dev = 0.0;
	int count = 0, dim1 = 0, dim2 = 0;

	array.get_dimension(dim1, dim2);
	// if you pass 0 for max and min, represent you want this function to find max and min by its own.
	// otherwise, this function won't help you to find max and min
	if (max == min && max == 0) {
		max = 0.0;
		min = 10000000.0;
		for (int i = 0; i < dim1; i++) {
			for (int j = 0; j < dim2; j++) {
				if (array.get_data(i, j) != 0) {
					if (array.get_data(i, j) < min) min = array.get_data(i, j);
					if (array.get_data(i, j) > max) max = array.get_data(i, j);
					sum += array.get_data(i, j);
					count++;
				}
			}
		}
	}
	else {
		for (int i = 0; i < dim1; i++) {
			for (int j = 0; j < dim2; j++) {
				if (array.get_data(i, j) != 0) {
					sum += array.get_data(i, j);
					count++;
				}
			}
		}
	}

	// mean value
	mean = sum / count;

	// var
	for (int i = 0; i < dim1; i++) {
		for (int j = 0; j < dim2; j++) {
			if (array.get_data(i, j) != 0) {
				variance += pow(array.get_data(i, j) - mean, 2);
			}
		}
	}
	variance /= count;

	// std
	std_dev = sqrt(variance);

	std::vector<cv::Vec3b> colorMat, colorMat_z;
	int picture_size = image_setup.img_size / image_setup.block_size;
	cv::Mat pic = cv::Mat::zeros(cv::Size(picture_size, picture_size), CV_8UC3);
	double scale = 255.0 / (double(max) - double(min));
	int color;
	for (int i = 0; i < picture_size; i++)
	{
		for (int j = 0; j < picture_size; j++)
		{
			if (array.get_data(i, j) != 0)
			{
				color = int(scale * (array.get_data(i, j) - min));
				pic.at<cv::Vec3b>(i, j) = cv::Vec3b(color, color, color);
			}

		}
	}

	cv::applyColorMap(pic, pic, 2);//��Ϥ��ܦ�ColorMap
	//�I���|�ܦ�����ҥH�n�N�I���ܦ^�զ�
	for (int i = 0; i < picture_size; i++)
	{
		for (int j = 0; j < picture_size; j++)
		{
			if (array.get_data(i, j) == 0)
				pic.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 255, 255);
		}
	}

	//�[�Wcolor bar
	int base_text = 0, tmp_max = max;
	while (tmp_max != 0)
	{
		tmp_max /= 10;
		base_text++;
	}

	int num_bar_w = (base_text + 4) * 10;
	int color_bar_w = 10;
	int vline = 10;
	int cb_top = 20;
	int cb_bottom = 20;

	cv::Mat pic_colorbar(cv::Size(pic.cols + num_bar_w + color_bar_w + vline, pic.rows),
		CV_8UC3, cv::Scalar(255, 255, 255)); //�إߤ@�Ӧ��dcolor_bar�Ŷ���Mat�Ӧs
	pic.copyTo(pic_colorbar(cv::Rect(0, 0, pic.cols, pic.rows)));

	cv::Mat color_bar(cv::Size(color_bar_w, pic.rows - (cb_top + cb_bottom)), CV_8UC3, cv::Scalar(255, 255, 255));

	//�إ�color_bar
	for (int i = 0; i < color_bar.rows; ++i)
	{
		for (int j = 0; j < color_bar_w; ++j)
		{
			int c = 255 - 255 * i / color_bar.rows;
			color_bar.at<cv::Vec3b>(i, j) = cv::Vec3b(c, c, c);
		}
	}
	color_bar.convertTo(color_bar, CV_8UC3);
	cv::applyColorMap(color_bar, color_bar, 2);

	//��Wcolor_bar�����
	int each_part = ceil((max - min) / 10.0);
	cv::Mat num_window(cv::Size(num_bar_w, pic.rows), CV_8UC3, cv::Scalar(255, 255, 255));
	for (int i = max; i >= int(min); i -= each_part)
	{
		int j = num_window.rows - ((float(i - min) / (max - min)) * color_bar.rows + cb_top);
		if (j == num_window.rows - cb_top) j = num_window.rows - cb_top - 1;
		cv::putText(num_window, std::to_string(i), cv::Point(5, j + 2), cv::FONT_ITALIC, 0.4, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
		cv::rectangle(num_window, cv::Point(0, j), cv::Point(3, j), cv::Scalar(0, 0, 0), 2);
	}

	num_window.copyTo(pic_colorbar(cv::Rect(pic.cols + vline + color_bar_w, 0, num_bar_w, pic.rows)));
	color_bar.copyTo(pic_colorbar(cv::Rect(pic.cols + vline, cb_top, color_bar_w, pic.rows - (cb_top + cb_bottom))));
	
	if (!stats_flag) {
		return pic_colorbar;
	}
	else {
		// �ק�o�̡G�W�[��ܲέp�ƾڪ��Ŷ�
		int stats_height = 60; // �έp�ƾ���ܰϰ쪺����
		cv::Mat pic_with_stats(cv::Size(pic_colorbar.cols, pic_colorbar.rows + stats_height),
			CV_8UC3, cv::Scalar(255, 255, 255));

		// �ƻs�즳���Ϲ���s��Mat��
		pic_colorbar.copyTo(pic_with_stats(cv::Rect(0, 0, pic_colorbar.cols, pic_colorbar.rows)));

		// �K�[�έp�ƾڤ奻�A�ϥΧ�p���r��j�p
		double font_scale = 0.5; // ��p�r��j�p
		int thickness = 1.5;
		int line_spacing = 13; // ��p�涡�Z

		cv::Point text_org(10, pic_colorbar.rows + 15);
		cv::putText(pic_with_stats,
			cv::format("Mean: %.2f", mean),
			text_org, cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), thickness);

		text_org.y += line_spacing;
		cv::putText(pic_with_stats,
			cv::format("Variance: %.2f", variance),
			text_org, cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), thickness);

		text_org.y += line_spacing;
		cv::putText(pic_with_stats,
			cv::format("Std Dev: %.2f", std_dev),
			text_org, cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), thickness);

		return pic_with_stats;
	}

}
cv::Mat color_map(const Array2D<int>& array, Image_setup image_setup, int max, int min, bool stats_flag)
{
	double sum = 0.0, mean, variance = 0.0, std_dev = 0.0;
	int count = 0, dim1 = 0, dim2 = 0;

	array.get_dimension(dim1, dim2);
	// if you pass 0 for max and min, represent you want this function to find max and min by its own.
	// otherwise, this function won't help you to find max and min
	if (max == min && max == 0) {
		max = 0.0;
		min = 10000000.0;
		for (int i = 0; i < dim1; i++) {
			for (int j = 0; j < dim2; j++) {
				if (array.get_data(i, j) != 0) {
					if (array.get_data(i, j) < min) min = array.get_data(i, j);
					if (array.get_data(i, j) > max) max = array.get_data(i, j);
					sum += array.get_data(i, j);
					count++;
				}
			}
		}
	}
	else {
		for (int i = 0; i < dim1; i++) {
			for (int j = 0; j < dim2; j++) {
				if (array.get_data(i, j) != 0) {
					sum += array.get_data(i, j);
					count++;
				}
			}
		}
	}

	// mean value
	mean = sum / count;

	// var
	for (int i = 0; i < dim1; i++) {
		for (int j = 0; j < dim2; j++) {
			if (array.get_data(i, j) != 0) {
				variance += pow(array.get_data(i, j) - mean, 2);
			}
		}
	}
	variance /= count;

	// std
	std_dev = sqrt(variance);

	std::vector<cv::Vec3b> colorMat, colorMat_z;
	int picture_size = image_setup.img_size / image_setup.block_size;
	cv::Mat pic = cv::Mat::zeros(cv::Size(picture_size, picture_size), CV_8UC3);
	double scale = 255.0 / (double(max) - double(min));
	int color;
	for (int i = 0; i < picture_size; i++)
	{
		for (int j = 0; j < picture_size; j++)
		{
			if (array.get_data(i, j) != 0)
			{
				color = int(scale * (array.get_data(i, j) - min));
				pic.at<cv::Vec3b>(i, j) = cv::Vec3b(color, color, color);
			}

		}
	}

	cv::applyColorMap(pic, pic, 2);//��Ϥ��ܦ�ColorMap
	//�I���|�ܦ�����ҥH�n�N�I���ܦ^�զ�
	for (int i = 0; i < picture_size; i++)
	{
		for (int j = 0; j < picture_size; j++)
		{
			if (array.get_data(i, j) == 0)
				pic.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 255, 255);
		}
	}

	//�[�Wcolor bar
	int base_text = 0, tmp_max = max;
	while (tmp_max != 0)
	{
		tmp_max /= 10;
		base_text++;
	}

	int num_bar_w = (base_text + 4) * 10;
	int color_bar_w = 10;
	int vline = 10;
	int cb_top = 20;
	int cb_bottom = 20;

	cv::Mat pic_colorbar(cv::Size(pic.cols + num_bar_w + color_bar_w + vline, pic.rows),
		CV_8UC3, cv::Scalar(255, 255, 255)); //�إߤ@�Ӧ��dcolor_bar�Ŷ���Mat�Ӧs
	pic.copyTo(pic_colorbar(cv::Rect(0, 0, pic.cols, pic.rows)));

	cv::Mat color_bar(cv::Size(color_bar_w, pic.rows - (cb_top + cb_bottom)), CV_8UC3, cv::Scalar(255, 255, 255));

	//�إ�color_bar
	for (int i = 0; i < color_bar.rows; ++i)
	{
		for (int j = 0; j < color_bar_w; ++j)
		{
			int c = 255 - 255 * i / color_bar.rows;
			color_bar.at<cv::Vec3b>(i, j) = cv::Vec3b(c, c, c);
		}
	}
	color_bar.convertTo(color_bar, CV_8UC3);
	cv::applyColorMap(color_bar, color_bar, 2);

	//��Wcolor_bar�����
	int each_part = ceil((max - min) / 10.0);
	cv::Mat num_window(cv::Size(num_bar_w, pic.rows), CV_8UC3, cv::Scalar(255, 255, 255));
	for (int i = max; i >= int(min); i -= each_part)
	{
		int j = num_window.rows - ((float(i - min) / (max - min)) * color_bar.rows + cb_top);
		if (j == num_window.rows - cb_top) j = num_window.rows - cb_top - 1;
		cv::putText(num_window, std::to_string(i), cv::Point(5, j + 2), cv::FONT_ITALIC, 0.4, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
		cv::rectangle(num_window, cv::Point(0, j), cv::Point(3, j), cv::Scalar(0, 0, 0), 2);
	}

	num_window.copyTo(pic_colorbar(cv::Rect(pic.cols + vline + color_bar_w, 0, num_bar_w, pic.rows)));
	color_bar.copyTo(pic_colorbar(cv::Rect(pic.cols + vline, cb_top, color_bar_w, pic.rows - (cb_top + cb_bottom))));
	
	if (!stats_flag) {
		return pic_colorbar;
	}
	else {
		// �ק�o�̡G�W�[��ܲέp�ƾڪ��Ŷ�
		int stats_height = 60; // �έp�ƾ���ܰϰ쪺����
		cv::Mat pic_with_stats(cv::Size(pic_colorbar.cols, pic_colorbar.rows + stats_height),
			CV_8UC3, cv::Scalar(255, 255, 255));

		// �ƻs�즳���Ϲ���s��Mat��
		pic_colorbar.copyTo(pic_with_stats(cv::Rect(0, 0, pic_colorbar.cols, pic_colorbar.rows)));

		// �K�[�έp�ƾڤ奻�A�ϥΧ�p���r��j�p
		double font_scale = 0.5; // ��p�r��j�p
		int thickness = 1.5;
		int line_spacing = 13; // ��p�涡�Z

		cv::Point text_org(10, pic_colorbar.rows + 15);
		cv::putText(pic_with_stats,
			cv::format("Mean: %.2f", mean),
			text_org, cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), thickness);

		text_org.y += line_spacing;
		cv::putText(pic_with_stats,
			cv::format("Variance: %.2f", variance),
			text_org, cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), thickness);

		text_org.y += line_spacing;
		cv::putText(pic_with_stats,
			cv::format("Std Dev: %.2f", std_dev),
			text_org, cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), thickness);

		return pic_with_stats;
	}

}

std::vector<std::array<int, 2>>  get_line_pixels(double x1, double y1, double x2, double y2, float& m, float& c)
{
	// FIX std::vector to array
	std::vector<std::array<int, 2>> results;
	// FIX add reserve for std::vector
	results.reserve(10);

	m = (y2 - y1) / (x2 - x1);
	c = y1 - m * x1;
	int y, x;
	if (x2 == x1)
	{
		x = x1;
		if (y1 > y2)
		{
			std::swap(y1, y2);
		}
		for (y = y1; y <= y2; y++)
		{
			results.emplace_back(std::array<int, 2>{x, y});
		}
		return results;
	}
	else if (x2 > x1)
	{
		x = floor(x1); y = floor(y1);
		results.emplace_back(std::array<int, 2>{x, y});
		for (x = x1 + 1; x < x2; x++)
		{
			y = floor(m * x + c);
			results.emplace_back(std::array<int, 2>{x, y});
		}
	}
	else
	{
		x = floor(x2); y = floor(y2);
		results.emplace_back(std::array<int, 2>{x, y});
		for (x = x2 + 1; x < x1; x++)
		{
			y = floor(m * x + c);
			results.emplace_back(std::array<int, 2>{x, y});
		}
	}
	if (y2 > y1)
	{
		for (y = y1 + 1; y < y2; y = y + 1)
		{
			x = floor((y - c) / m);
			results.emplace_back(std::array<int, 2>{x, y});
		}
	}
	else
	{
		for (y = y2 + 1; y < y1; y = y + 1)
		{
			x = floor((y - c) / m);
			results.emplace_back(std::array<int, 2>{x, y});
		}
	}

	sort(results.begin(), results.end());
	auto last = unique(results.begin(), results.end());
	results.erase(last, results.end());

	return results;
}
void get_line_coordinate(double x1, double y1, double x2, double y2, std::vector<std::vector<double>>(&line_coordinate)[SIZE][SIZE])
{
	float m, c;
	// FIX std::vector to array
	std::vector<std::array<int, 2>> pixels;
	pixels = get_line_pixels(x1, y1, x2, y2, m, c);
	//print2D(pixels);
	int x, y;
	float temp_x1, temp_y1, temp_x2, temp_y2;
	std::vector<double> temp;
	// FIX add reserver for std::vector
	temp.reserve(4);

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
	for (int i = 0; i < pixels.size(); i++)
	{
		x = pixels[i][0], y = pixels[i][1];
		temp.clear();
		//���O�Nx=pixels[i][0] x=pixels[i][0] y = pixels[i][1] y = pixels[i][1]+1�a�Jlinear equation y=mx+c
		temp_x1 = (y - c) / m;
		temp_y1 = m * x + c;
		temp_x2 = (y + 1 - c) / m;
		temp_y2 = m * (x + 1) + c;
		//�ˬd�O�_�b��pixel���ˬd�|�Ӧ�m
		if (m == INFINITY || m == -INFINITY)
		{
			temp.push_back(double(x));
			temp.push_back(double(y));
			temp.push_back(double(x));
			temp.push_back(double(y + 1));
		}
		else
		{
			if (temp_y1 <= y + 1 && temp_y1 >= y)
			{
				temp.push_back({ double(x) });
				temp.push_back({ double(temp_y1) });

			}
			if (temp_y2 <= y + 1 && temp_y2 >= y)
			{
				temp.push_back({ double(x + 1) });
				temp.push_back({ double(temp_y2) });

			}
			if (temp_x1 <= x + 1 && temp_x1 >= x)
			{
				temp.push_back({ double(temp_x1) });
				temp.push_back({ double(y) });
			}
			if (temp_x2 <= x + 1 && temp_x2 >= x)
			{
				temp.push_back({ double(temp_x2) });
				temp.push_back({ double(y + 1) });
			}
		}
		//print_std::vector(temp);
		if (temp.size() == 4)
		{
			line_coordinate[y][x].push_back(temp);
		}

	}

}
void get_all_line_coordinate(Locus_coordinate locus_coordinate, Image_setup image_setup, std::vector<std::vector<double>>(&line_coordinate)[SIZE][SIZE])
{
	const int img_size = image_setup.img_size;
	const int block_size = image_setup.block_size;
	int center = img_size / 2;
	double row = center, col = center;

	auto start_time = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < img_size; i++)
	{
		for (int j = 0; j < img_size; j++)
		{
			line_coordinate[i][j].reserve(1000);
		}
	}
	for (int i = 0; i < image_setup.diamond_size; i++)
	{
		// printf("  total=%d ,now=%d , percent = %f \r", image_setup.diamond_size, i, i * 100 / float(image_setup.diamond_size));

		for (int j = 0; j < locus_coordinate.x[0].size() - 1; j++)
		{
			double x1, y1, x2, y2;
			x1 = (locus_coordinate.x[i][j] + row);//把座標位移到pixel座標左上角為(0,0)
			if (x1 < 0) x1 = 0;
			y1 = (-locus_coordinate.y[i][j] + col);
			if (y1 < 0) y1 = 0;
			x2 = (locus_coordinate.x[i][j + 1] + row);
			if (x2 < 0) x2 = 0;
			y2 = (-locus_coordinate.y[i][j + 1] + col);
			if (y2 < 0) y2 = 0;
			get_line_coordinate(x1, y1, x2, y2, line_coordinate);
		}
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

	// std::cout << "Time taken by the get_all_line_coordinate: " << duration.count() << " milliseconds" << std::endl;

}
// 計算元素總數並返回 int
int count_elements(const std::vector<std::vector<double>> (&line_coordinate)[SIZE][SIZE]) {
    size_t total = 0;
    for (const auto& dim1 : line_coordinate) {
        for (const auto& dim2 : dim1) {
            total += static_cast<int>(dim2.size() * 4);
        }
    }
    
    return total;
}
bool validate_conversion(const std::vector<std::vector<double>> (&line_coordinate)[SIZE][SIZE], const PixelLineSegments& pixelData) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {

			int original_size_value = line_coordinate[i][j].size();
			int converted_size_value = pixelData.get_pixel_line_num(i, j);
			if (original_size_value != converted_size_value) {
				std::cerr << "Mismatch size at (" << i << ", " << j << "): "
					<< "original=" << original_size_value << ", converted=" << converted_size_value << std::endl;
				return false;
			}

            for (int k = 0; k < line_coordinate[i][j].size(); ++k) {
                for (int l = 0; l < line_coordinate[i][j][k].size(); ++l) {
                    double original_value = line_coordinate[i][j][k][l];
                    double converted_value = pixelData.get_data(i, j, k, l);
                    if (std::abs(original_value - converted_value) > std::numeric_limits<double>::epsilon()) {
                        std::cerr << "Mismatch vsalue at (" << i << ", " << j << ", " << k << ", " << l << "): "
                                  << "original=" << original_value << ", converted=" << converted_value << std::endl;
                        return false;
                    }
                }
            }

        }
    }
    return true;
}
PixelLineSegments convert_array4d_to_pixelData(const std::vector<std::vector<double>> (&line_coordinate)[SIZE][SIZE]) {
    int total_elements = count_elements(line_coordinate);
    PixelLineSegments pixelData(total_elements, SIZE);
    
    int size_index = 0, i = 0, j = 0;
    for (i = 0; i < SIZE; ++i) {

		// printf("  total=%d ,now=%d , percent = %f \r", SIZE, i, float(i * 100 / SIZE));

        for (j = 0; j < SIZE; ++j) {

			pixelData.set_pixel_line_num(i, j, line_coordinate[i][j].size());
			pixelData.set_pixel_start_index(i, j, size_index);
			size_index += line_coordinate[i][j].size();

            for (int k = 0; k < line_coordinate[i][j].size(); ++k) {
                for (int l = 0; l < line_coordinate[i][j][k].size(); ++l) {
                    pixelData.set_data(i, j, k, l, line_coordinate[i][j][k][l]);
                }
            }
        }
    }
    
	// check if the conversion is successful
	/*std::cout << "Conversion: " 
		<< (validate_conversion(line_coordinate, pixelData) ? "Successed, there is no different between two array" : "Failed, there is different between two array") 
		<< std::endl;*/

    return pixelData;
}

void trajectory_distance(const int start, const int end, const PixelLineSegments& pixelData, PAP_distribution& mrr_pixel_level) {
	
	for (int i = start; i < end; ++i) {

		// printf("  total=%d ,now=%d , percent = %f \r", SIZE, i, float(i * 100 / SIZE));

		for (int j = 0; j < SIZE; ++j) {

			double distance = 0.0;
			for (int k = 0; k < pixelData.get_pixel_line_num(i, j); ++k) {

				double x1 = pixelData.get_data(i, j, k, 0);
				double y1 = pixelData.get_data(i, j, k, 1);
				double x2 = pixelData.get_data(i, j, k, 2);
				double y2 = pixelData.get_data(i, j, k, 3);
				distance += sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
			}

			mrr_pixel_level.set_distance(i, j, distance);
		}
	}

}

void multi_thread_trajectory_distance(const PixelLineSegments& pixelData, PAP_distribution& mrr_pixel_level) {

	auto start_time = std::chrono::high_resolution_clock::now();

	const int num_threads = 25;
	std::thread threads[num_threads];

	int start = 0;
	int end = SIZE / num_threads;

	for (int i = 0; i < num_threads; ++i) {
		threads[i] = std::thread(trajectory_distance, start, end, std::ref(pixelData), std::ref(mrr_pixel_level));
		start = end;
		end += SIZE / num_threads;
	}

	for (int i = 0; i < num_threads; ++i) {
		threads[i].join();
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

	// std::cout << "Time taken by the multi-thread trajectory distance: " << duration.count() << " milliseconds" << std::endl;

}

bool check_overlap(double* line1, double* line2) {

	//line1(x1, y1, x2, y2), line2(x1, y1, x2, y2)
	int range_x = line1[0];
	int range_y = line1[1];

	double a1 = line1[3] - line1[1]; // y2-y1
	double b1 = line1[0] - line1[2]; // x2-x1
	double c1 = a1 * line1[0] + b1 * line1[1];

	double a2 = line2[3] - line2[1]; // y
	double b2 = line2[0] - line2[2]; // x
	double c2 = a2 * line2[0] + b2 * line2[1];

	double determinant = a1 * b2 - a2 * b1;

	if (determinant == 0)
	{
		return false;
	}
	else
	{
		double x = (b2 * c1 - b1 * c2) / determinant;
		double y = (a1 * c2 - a2 * c1) / determinant;

		if (x >= range_x && x < range_x + 1 && y >= range_y && y < range_y + 1)
			return true;
		else return false;
	}

}

double calculate_overlap_angle(double* line1, double* line2) {

	// line = { x1, y1, x2, y2 }
	double* vector1 = new double[2]{ line1[2] - line1[0], line1[3] - line1[1] };
	double* vector2 = new double[2]{ line2[2] - line2[0], line2[3] - line2[1] };

	double dotProduct = vector1[0] * vector2[0] + vector1[1] * vector2[1];

	// Avoid unnecessary square root calculations

	double magnitude1_squared = vector1[0] * vector1[0] + vector1[1] * vector1[1];
	double magnitude2_squared = vector2[0] * vector2[0] + vector2[1] * vector2[1];

	delete[] vector1;
	delete[] vector2;

	// FIX combine two times of sqrt to one times
	double cosineTheta = dotProduct / sqrt(magnitude1_squared * magnitude2_squared);

	double thetaRadians = acos(cosineTheta);

	double thetaDegrees = thetaRadians * 180.0 / M_PI;
	if (thetaDegrees > 90) {
		return 180 - thetaDegrees;
	}
	else {
		return thetaDegrees;
	}
}

void intersection_and_angle(const int start, const int end, const PixelLineSegments& pixelData, PAP_distribution& mrr_pixel_level) {

	double* line1 = new double[4];
	double* line2 = new double[4];
	
	for (int i = start; i < end; ++i) {
		for (int j = 0; j < SIZE; ++j) {

			int overlap_num = 0;
			double angle = 0.0;
			for (int k = 0; k < pixelData.get_pixel_line_num(i, j); ++k) {
				for (int l = k + 1; l < pixelData.get_pixel_line_num(i, j); ++l) {

					line1[0] = pixelData.get_data(i, j, k, 0);
					line1[1] = pixelData.get_data(i, j, k, 1);
					line1[2] = pixelData.get_data(i, j, k, 2);
					line1[3] = pixelData.get_data(i, j, k, 3);

					line2[0] = pixelData.get_data(i, j, l, 0);
					line2[1] = pixelData.get_data(i, j, l, 1);
					line2[2] = pixelData.get_data(i, j, l, 2);
					line2[3] = pixelData.get_data(i, j, l, 3);

					if (check_overlap(line1, line2)) {
						angle += calculate_overlap_angle(line1, line2);
						++overlap_num;
					}
				}
			}

			if (overlap_num != 0) {
				mrr_pixel_level.set_angle(i, j, angle / overlap_num);
			}
			
			mrr_pixel_level.set_intersection(i, j, overlap_num);
		}
	}

	delete[] line1;
	delete[] line2;

}

void multi_thread_intersection_and_angle(const PixelLineSegments& pixelData, PAP_distribution& mrr_pixel_level) {

	auto start_time = std::chrono::high_resolution_clock::now();

	const int num_threads = 250;
	std::thread threads[num_threads];

	int start = 0;
	int end = SIZE / num_threads;

	for (int i = 0; i < num_threads; ++i) {
		threads[i] = std::thread(intersection_and_angle, start, end, std::ref(pixelData), std::ref(mrr_pixel_level));
		start = end;
		end += SIZE / num_threads;
	}

	for (int i = 0; i < num_threads; ++i) {
		threads[i].join();
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

	// std::cout << "Time taken by the multi-thread intersection and angle: " << duration.count() << " milliseconds" << std::endl;

}

void PAP_calculation(PAP_distribution& mrr_pixel_level) {

	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			double mrr_value = 
				0.2 * mrr_pixel_level.get_intersection(i, j) 
				+ 0 * mrr_pixel_level.get_angle(i, j) 
				+ 0.8 * mrr_pixel_level.get_distance(i, j);

			mrr_pixel_level.set_mrr(i, j, mrr_value);
		}
	}
}

PAP_distribution pixel2block_level(const PAP_distribution& mrr_pixel_level, const Image_setup& image_info) {

	int block_size = image_info.block_size;

	PAP_distribution mrr_block_level(SIZE / block_size);

	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			mrr_block_level.set_intersection(
				i / block_size, 
				j / block_size, 
				mrr_pixel_level.get_intersection(i, j) + mrr_block_level.get_intersection(i / block_size, j / block_size)
			);
			mrr_block_level.set_angle(
				i / block_size, 
				j / block_size, 
				mrr_pixel_level.get_angle(i, j) + mrr_block_level.get_angle(i / block_size, j / block_size)
			);
			mrr_block_level.set_distance(
				i / block_size, 
				j / block_size, 
				mrr_pixel_level.get_distance(i, j) + mrr_block_level.get_distance(i / block_size, j / block_size)
			);
			mrr_block_level.set_mrr(
				i / block_size, 
				j / block_size, 
				mrr_pixel_level.get_mrr(i, j) + mrr_block_level.get_mrr(i / block_size, j / block_size)
			);
		}
	}

	for (int i = 0; i < SIZE / block_size; ++i) {
		for (int j = 0; j < SIZE / block_size; ++j) {
			mrr_block_level.set_angle(i, j, mrr_block_level.get_angle(i, j) / (block_size * block_size));
		}
	}

	return mrr_block_level;

}

