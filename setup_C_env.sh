#!/bin/bash

# Error handling function
handle_error() {
    echo "Error: $1"
    exit 1
}

# Check command execution result
check_command() {
    if [ $? -ne 0 ]; then
        handle_error "$1"
    fi
}

# Ask whether to use sudo
read -p "Use sudo to execute commands? (y/n): " use_sudo
if [[ $use_sudo == "y" ]]; then
    CMD="sudo"
else
    CMD=""
fi

# Update package list
echo "Updating package list..."
$CMD apt update
check_command "Unable to update package list"

# Install necessary packages
echo "Installing necessary packages..."
$CMD apt install software-properties-common gcc-12 g++-12 build-essential gdb -y
check_command "Unable to install necessary packages"

# Set up gcc and g++ alternative versions
echo "Setting up gcc and g++ alternative versions..."
$CMD update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 120 --slave /usr/bin/g++ g++ /usr/bin/g++-12
check_command "Unable to set up gcc and g++ alternative versions"

# Check gcc version
gcc_version=$(gcc --version | head -n1)
echo "gcc version: $gcc_version"

# Install OpenCV dependencies
echo "Installing OpenCV dependencies..."
$CMD apt install build-essential cmake git pkg-config libgtk-3-dev \
    libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
    libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev \
    gfortran openexr libatlas-base-dev python3-dev python3-numpy \
    libtbb2 libtbb-dev libdc1394-dev libopencv-imgcodecs-dev -y
check_command "Unable to install OpenCV dependencies"

# Create OpenCV build directory
echo "Creating OpenCV build directory..."
mkdir -p ~/opencv_build && cd ~/opencv_build
check_command "Unable to create or enter OpenCV build directory"

# Clone OpenCV repository
echo "Cloning OpenCV repository..."
git clone https://github.com/opencv/opencv.git
check_command "Unable to clone OpenCV repository"
git clone https://github.com/opencv/opencv_contrib.git
check_command "Unable to clone OpenCV contrib repository"

# Set up OpenCV build
echo "Setting up OpenCV build..."
cd ~/opencv_build/opencv
mkdir -p build && cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_C_EXAMPLES=ON \
    -D INSTALL_PYTHON_EXAMPLES=ON \
    -D OPENCV_GENERATE_PKGCONFIG=ON \
    -D OPENCV_EXTRA_MODULES_PATH=~/opencv_build/opencv_contrib/modules \
    -D BUILD_EXAMPLES=ON ..
check_command "CMake configuration failed"

# Compile OpenCV
echo "Compiling OpenCV..."
make -j$(nproc)
check_command "OpenCV compilation failed"

# Install OpenCV
echo "Installing OpenCV..."
$CMD make install
check_command "OpenCV installation failed"

$CMD ldconfig
check_command "ldconfig failed"

# Check OpenCV version
opencv_version=$(pkg-config --modversion opencv4)
echo "OpenCV version: $opencv_version"

# Output OpenCV path
opencv_path=$(pkg-config --cflags opencv4)
echo "OpenCV path: $opencv_path"

echo "OpenCV installation completed!"