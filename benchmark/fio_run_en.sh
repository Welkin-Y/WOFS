#!/bin/bash

# Define variables
TEST_DIR="/tmp/wofs_test"
IMAGE_FILE="/tmp/wofs_image"
MOUNT_POINT_EN="/tmp/mnt_en"
MOUNT_POINT="/tmp/mnt"
RESULTS_DIR="result"
TEST_FILE_SIZE="10M" # Adjust file size as needed
NUM_JOBS=4 # Number of parallel jobs to run, adjust as needed

# Create test directory and files
echo "Creating test directory and files..."
mkdir -p "$TEST_DIR"
for i in {0..200}; do
    dd if=/dev/zero of="$TEST_DIR"/file"$i".txt bs=1M count=10
done

#Generate WOFS image
# echo "Generating WOFS image..."
# ../build/wofs gen "$TEST_DIR" "$IMAGE_FILE"
# echo "Mounting WOFS directory..."
# mkdir -p "$MOUNT_POINT"
# ../build/wofs mount "$IMAGE_FILE" "$MOUNT_POINT"


echo "Generating WOFS_encrpyted image..."
../build/wofs gen -e "$TEST_DIR" "$IMAGE_FILE" 

echo "Mounting WOFS_encrpyted directory..."
mkdir -p "$MOUNT_POINT_EN"
../build/wofs mount -e "$IMAGE_FILE.enc" "$MOUNT_POINT_EN"

# Ensure fio is installed
if ! command -v fio &> /dev/null; then
    echo "fio is not installed. Please install it before running this script."
    exit 1
fi

# Ensure fio results directory exists
mkdir -p "$RESULTS_DIR"


# for i in {1..10}; do
#         sudo bash -c "sync; echo 3 > /proc/sys/vm/drop_caches"
#         fio  --directory="$TEST_DIR" --readwrite=read --ioengine=psync --invalidate=1 --readonly --name=read_seq --bs=4k  --iodepth=1 --size="$TEST_FILE_SIZE" --output="$RESULTS_DIR/org_sread_"$i"_all.txt"

#         sudo bash -c "sync; echo 3 > /proc/sys/vm/drop_caches"
#         fio  --directory="$MOUNT_POINT" --readwrite=read --ioengine=psync --invalidate=1 --readonly --name=read_seq --bs=4k  --iodepth=1 --size="$TEST_FILE_SIZE" --output="$RESULTS_DIR/mnt_sread_"$i"_all.txt"


        
        
#         # Random read test   
#         sudo bash -c "sync; echo 3 > /proc/sys/vm/drop_caches"  
#         fio  --directory="$TEST_DIR" --readwrite=randread --ioengine=psync --invalidate=1 --readonly --name=read_rand --bs=4k  --iodepth=1 --size="$TEST_FILE_SIZE" --output="$RESULTS_DIR/org_rread_"$i"_all.txt"
        
#         sudo bash -c "sync; echo 3 > /proc/sys/vm/drop_caches"
#         fio  --directory="$MOUNT_POINT" --readwrite=randread --ioengine=psync --invalidate=1 --readonly --name=read_rand --bs=4k  --iodepth=1 --size="$TEST_FILE_SIZE" --output="$RESULTS_DIR/mnt_rread_"$i"_all.txt"
       
#     done


for ((n = 0; n <= 200; n += 20)); do
    
    for i in {1..10}; do
        # Sequential read test
       
        sudo bash -c "sync; echo 3 > /proc/sys/vm/drop_caches"
        fio  --filename="$MOUNT_POINT_EN"/file"$n".txt --readwrite=read --ioengine=psync --invalidate=1 --readonly --name=read_seq --bs=4k  --iodepth=1 --size="$TEST_FILE_SIZE" --output="$RESULTS_DIR/en_sread_"$i"_file_"$n".txt"

        
        
        # Random read test   
        sudo bash -c "sync; echo 3 > /proc/sys/vm/drop_caches"
        fio  --filename="$MOUNT_POINT_EN"/file"$n".txt --readwrite=randread --ioengine=psync --invalidate=1 --readonly --name=read_rand --bs=4k  --iodepth=1 --size="$TEST_FILE_SIZE" --output="$RESULTS_DIR/en_rread_"$i"_file_"$n".txt"
        
    done
done

echo "Benchmarking complete. Results saved in $RESULTS_DIR."

rm -rf $TEST_DIR
# umount $MOUNT_POINT
umount $MOUNT_POINT_EN
# rm -rf $MOUNT_POINT
rm -rf $MOUNT_POINT_EN
# rm $IMAGE_FILE
rm "$IMAGE_FILE.enc"
