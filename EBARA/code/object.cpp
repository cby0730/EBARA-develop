#include "object.h"

// ====================================Diamonds_coordinate====================================
void Diamonds_coordinate::Load_Diamond_Coordinates_Swingarm(std::string path)
{
	std::ofstream inFile(path, std::ios::in);

	if (!inFile)
	{
		std::cout << "Coordinates load error!" << std::endl;
		exit(1);
	}
	std::string value;
	std::string tmp;
	float tmpf;
	int x = 0;
	std::ifstream infile(path);
	while (infile.good())
	{
		getline(infile, value, '\n');
		if (value != "\0")
		{
			x = value.find(',');
			tmpf = stod(tmp.assign(value, 0, x));
			this->x.push_back((tmpf));
			tmpf = stod(tmp.assign(value, x + 1, 20));
			this->y.push_back(tmpf);
		}
	}
	std::cout << "Coordinates load finish" << std::endl;
	infile.close();
}
void Diamonds_coordinate::Cart2Pol()
{
	int n = this->x.size();
	this->theta.assign(n, 0);
	this->rho.assign(n, 0);

	for (size_t i = 0; i < n; i++)
	{
		this->theta[i] = atan2(this->x[i], this->y[i]);
		this->rho[i] = sqrt(pow(this->x[i], 2) + pow(this->y[i], 2));
	}
	std::cout << "Done Cart2Pol" << std::endl;
}

// ====================================CMP_Parameter=====================================
void CMP_Parameter::print_parameter()
{
	std::cout << "pad speed: " << np << " dresser speed:" << nd << " dresser_move_speed:" << vs << " cycle:" << cycle << std::endl;
}

// ====================================PixelLineSegments implementation====================================
int PixelLineSegments::index(int i, int j, int k, int l) const {
    return (data_index[i * pixel_size + j] + k) * 4 + l;
}

PixelLineSegments::PixelLineSegments() : total_size(0), pixel_size(0), data(nullptr), data_index(nullptr), line_num(nullptr) {}

PixelLineSegments::PixelLineSegments(int total_size, int pixel_size) : total_size(total_size), pixel_size(pixel_size) {
    data = (double*)calloc(total_size, sizeof(double));
    if (data == nullptr) {
        throw std::bad_alloc();
    }
    data_index = (int*)calloc(pixel_size * pixel_size, sizeof(int));
    if (data_index == nullptr) {
        throw std::bad_alloc();
    }
    line_num = (int*)calloc(pixel_size * pixel_size, sizeof(int));
    if (line_num == nullptr) {
        throw std::bad_alloc();
    }
}

PixelLineSegments::~PixelLineSegments() {
    std::free(line_num);
    std::free(data_index);
    std::free(data);
}

double PixelLineSegments::get_data(int i, int j, int k, int l) const {
    if (i < 0 || i >= pixel_size || j < 0 || j >= pixel_size || k < 0 || k >= get_pixel_line_num(i, j) || l < 0 || l >= 4) {
        throw std::out_of_range("Index out of bounds");
    }
    return data[index(i, j, k, l)];
}

void PixelLineSegments::set_data(int i, int j, int k, int l, double value) {
    if (get_pixel_line_num(i, j) == 0) {
        throw std::out_of_range("Index out of bounds. Maybe you forget to set pixel's line number.");
    }
    if (i < 0 || i >= pixel_size || j < 0 || j >= pixel_size || k < 0 || k >= get_pixel_line_num(i, j) || l < 0 || l >= 4) {
        throw std::out_of_range("Index out of bounds");
    }
    data[index(i, j, k, l)] = value;
}

int PixelLineSegments::get_pixel_line_num(int i, int j) const {
    if (i < 0 || i >= pixel_size || j < 0 || j >= pixel_size) {
        throw std::out_of_range("Index out of bounds");
    }
    return line_num[i * pixel_size + j];
}

void PixelLineSegments::set_pixel_line_num(int i, int j, int value) {
    if (i < 0 || i >= pixel_size || j < 0 || j >= pixel_size) {
        throw std::out_of_range("Index out of bounds");
    }
    line_num[i * pixel_size + j] = value;
}

void PixelLineSegments::set_pixel_start_index(int i, int j, int value) {
    if (i < 0 || i >= pixel_size || j < 0 || j >= pixel_size) {
        throw std::out_of_range("Index out of bounds");
    }
    data_index[i * pixel_size + j] = value;
}

// ====================================Array2D implementation====================================
template<typename T>
int Array2D<T>::index(int i, int j) const {
    return i * dim2 + j;
}

template<typename T>
Array2D<T>::Array2D() : data(nullptr), dim1(0), dim2(0) {}

template<typename T>
Array2D<T>::Array2D(int dim1, int dim2) : dim1(dim1), dim2(dim2) {
    data = static_cast<T*>(calloc(dim1 * dim2, sizeof(T)));
    if (data == nullptr) {
        throw std::bad_alloc();
    }
}

template<typename T>
Array2D<T>::~Array2D() {
    std::free(data);
    dim1 = 0;
    dim2 = 0;
}

template<typename T>
Array2D<T>::Array2D(const Array2D& other) : dim1(other.dim1), dim2(other.dim2) {
    data = static_cast<T*>(calloc(dim1 * dim2, sizeof(T)));
    if (data == nullptr) {
        throw std::bad_alloc();
    }
    std::copy(other.data, other.data + dim1 * dim2, data);
}

// Copy assignment operator
template<typename T>
Array2D<T>& Array2D<T>::operator=(const Array2D& other) {
    if (this != &other) {
        std::free(data);
        dim1 = other.dim1;
        dim2 = other.dim2;
        data = static_cast<T*>(calloc(dim1 * dim2, sizeof(T)));
        if (data == nullptr) {
            throw std::bad_alloc();
        }
        std::copy(other.data, other.data + (dim1 * dim2), data);
    }
    return *this;
}

template<typename T>
T Array2D<T>::get_data(int i, int j) const {
    if (i < 0 || i >= dim1 || j < 0 || j >= dim2) {
        throw std::out_of_range("Index out of bounds");
    }
    return data[index(i, j)];
}

template<typename T>
void Array2D<T>::set_data(int i, int j, T value) {
    if (i < 0 || i >= dim1 || j < 0 || j >= dim2) {
        throw std::out_of_range("Index out of bounds");
    }
    data[index(i, j)] = value;
}

template<typename T>
void Array2D<T>::get_dimension(int& dim1, int& dim2) const {
    dim1 = this->dim1;
    dim2 = this->dim2;
}

// Explicit template instantiations
template class Array2D<int>;
template class Array2D<double>;

// ====================================PAP_distribution implementation====================================
PAP_distribution::PAP_distribution(int SIZE) : SIZE(SIZE), intersection(SIZE, SIZE), angle(SIZE, SIZE), distance(SIZE, SIZE), mrr(SIZE, SIZE) {}

int PAP_distribution::get_size() const {
    return SIZE;
}

void PAP_distribution::set_intersection(int i, int j, int value) {
    if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
        throw std::out_of_range("Index out of bounds");
    }
    intersection.set_data(i, j, value);
}

int PAP_distribution::get_intersection(int i, int j) const {
    if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
        throw std::out_of_range("Index out of bounds");
    }
    return intersection.get_data(i, j);
}

Array2D<int> PAP_distribution::get_intersection_array() const {
    return intersection;
}

void PAP_distribution::set_angle(int i, int j, double value) {
    if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
        throw std::out_of_range("Index out of bounds");
    }
    angle.set_data(i, j, value);
}

double PAP_distribution::get_angle(int i, int j) const {
    if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
        throw std::out_of_range("Index out of bounds");
    }
    return angle.get_data(i, j);
}

Array2D<double> PAP_distribution::get_angle_array() const {
    return angle;
}

void PAP_distribution::set_distance(int i, int j, double value) {
    if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
        throw std::out_of_range("Index out of bounds");
    }
    distance.set_data(i, j, value);
}

double PAP_distribution::get_distance(int i, int j) const {
    if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
        throw std::out_of_range("Index out of bounds");
    }
    return distance.get_data(i, j);
}

Array2D<double> PAP_distribution::get_distance_array() const {
    return distance;
}

void PAP_distribution::set_mrr(int i, int j, double value) {
    if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
        throw std::out_of_range("Index out of bounds");
    }
    mrr.set_data(i, j, value);
}

double PAP_distribution::get_mrr(int i, int j) const {
    if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
        throw std::out_of_range("Index out of bounds");
    }
    return mrr.get_data(i, j);
}

Array2D<double> PAP_distribution::get_mrr_array() const {
    return mrr;
}