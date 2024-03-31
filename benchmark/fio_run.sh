#!/bin/bash

# Define variables
TEST_DIR="wofs_test"
IMAGE_FILE="wofs_image.img"
MOUNT_POINT="mnt"
RESULTS_DIR="result"
TEST_FILE_SIZE="1G" # Adjust file size as needed
NUM_JOBS=4 # Number of parallel jobs to run, adjust as needed

# Create test directory and files
echo "Creating test directory and files..."
mkdir -p "$TEST_DIR"
cd "$TEST_DIR" || exit
for i in {1..10}; do
    dd if=/dev/zero of=file"$i".txt bs=1M count=10
done
cd ..

# Generate WOFS image
echo "Generating WOFS image..."
./wofs -g "$TEST_DIR" "$IMAGE_FILE"

# Ensure mount point exists
if [ ! -d "$MOUNT_POINT" ]; then
    echo "Mount point $MOUNT_POINT does not exist."
    exit 1
fi

# Ensure fio is installed
if ! command -v fio &> /dev/null; then
    echo "fio is not installed. Please install it before running this script."
    exit 1
fi

# Ensure fio results directory exists
mkdir -p "$RESULTS_DIR"

# Sequential read test
echo "Running sequential read test..."
fio --name=read_seq --filename="$MOUNT_POINT/file1.txt" --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=read --runtime=30s --time_based --output="$RESULTS_DIR/sequential_read.txt"

# Random read test
echo "Running random read test..."
fio --name=read_rand --filename="$MOUNT_POINT/file1.txt" --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=randread --runtime=30s --time_based --output="$RESULTS_DIR/random_read.txt"

echo "Benchmarking complete. Results saved in $RESULTS_DIR."
