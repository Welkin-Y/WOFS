#!/bin/bash

# Define variables
TEST_DIR="/tmp/wofs_test"
IMAGE_FILE="/tmp/wofs_image"
IMAGE_FILE_en="/tmp/wofs_image_en"
MOUNT_POINT="/tmp/mnt"
RESULTS_DIR="result"
TEST_FILE_SIZE="1M" # Adjust file size as needed
NUM_JOBS=4 # Number of parallel jobs to run, adjust as needed

# Create test directory and files
echo "Creating test directory and files..."
mkdir -p "$TEST_DIR"
# cd "$TEST_DIR" || exit
for i in {0..1000}; do
    dd if=/dev/zero of="$TEST_DIR"/file"$i".txt bs=1K count=10
done
# cd ..

#Generate WOFS image
echo "Generating WOFS image..."
../build/wofs gen "$TEST_DIR" "$IMAGE_FILE"
echo "Mounting WOFS directory..."
mkdir -p "$MOUNT_POINT"
../build/wofs mount "$IMAGE_FILE" "$MOUNT_POINT"


# echo "Generating WOFS_encrpyted image..."
# ../build/wofs gen -e "$TEST_DIR" "$IMAGE_FILE" <<EOF
# 123456
# EOF
# echo "Mounting WOFS_encrpyted directory..."
# mkdir -p "$MOUNT_POINT_EN"
# ../build/wofs mount -e "$IMAGE_FILE.enc" "$MOUNT_POINT_EN"<<EOF
# 123456
# EOF

# Ensure fio is installed
if ! command -v fio &> /dev/null; then
    echo "fio is not installed. Please install it before running this script."
    exit 1
fi

# Ensure fio results directory exists
mkdir -p "$RESULTS_DIR"

mkdir -p "/tmp/t1"

for i in {1..5}; do
        fio --directory="/tmp/t1" --name=read_seq --filename="$TEST_DIR"/* --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=read --output="$RESULTS_DIR"/org_sread_"$i"_all.txt

        fio --directory="/tmp/t1" --name=read_seq --filename="$MOUNT_POINT"/* --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=read  --output="$RESULTS_DIR"/mnt_sread_"$i"_all.txt

        # fio --directory="/tmp/t1" --name=read_seq --filename="$MOUNT_POINT_EN"/* --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=read  --output="$RESULTS_DIR"/en_sread_"$i"_all.txt

        
        # Random read test
        # echo "Running random read test..."
        # echo 3 > /proc/sys/vm/drop_caches
        fio --directory="/tmp/t1" --name=read_rand --filename="$TEST_DIR"/* --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=randread --output="$RESULTS_DIR"/org_rread_"$i"_all.txt
        
        fio --directory="/tmp/t1" --name=read_rand --filename="$MOUNT_POINT"/* --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=randread  --output="$RESULTS_DIR"/mnt_rread_"$i"_all.txt

        # fio --directory="/tmp/t1" --name=read_rand --filename="$MOUNT_POINT_EN"/* --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=randread  --output="$RESULTS_DIR"/en_rread_"$i"_all.txt
       
    done


for ((n = 0; n <= 1000; n += 200)); do
    
    for i in {1..5}; do
        # Sequential read test
        fio --directory="/tmp/t1" --name=read_seq --filename="$TEST_DIR"/file"$n".txt --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=read --output="$RESULTS_DIR/org_sread_"$i"_file_"$n".txt"
        
        fio --directory="/tmp/t1" --name=read_seq --filename="$MOUNT_POINT"/file"$n".txt --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=read  --output="$RESULTS_DIR/mnt_sread_"$i"_file_"$n".txt"

        # fio --directory="/tmp/t1" --name=read_seq --filename="$MOUNT_POINT_EN"/file"$n".txt --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=read  --output="$RESULTS_DIR/en_sread_"$i"_file_"$n".txt"
        
        
        # Random read test     
        fio --directory="/tmp/t1" --name=read_rand --filename="$TEST_DIR"/file"$n".txt --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=randread --output="$RESULTS_DIR/org_rread_"$i"_file_"$n".txt"
        
        fio --directory="/tmp/t1" --name=read_rand --filename="$MOUNT_POINT"/file"$n".txt --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=randread  --output="$RESULTS_DIR/mnt_rread_"$i"_file_"$n".txt"
        
        # fio --directory="/tmp/t1" --name=read_rand --filename="$MOUNT_POINT_EN"/file"$n".txt --bs=4k --iodepth=64 --size="$TEST_FILE_SIZE" --readwrite=randread  --output="$RESULTS_DIR/en_rread_"$i"_file_"$n".txt"

    done
done

echo "Benchmarking complete. Results saved in $RESULTS_DIR."

rm -rf $TEST_DIR
umount $MOUNT_POINT
# umount $MOUNT_POINT_EN
rm -rf $MOUNT_POINT
# rm -rf $MOUNT_POINT_EN
rm $IMAGE_FILE
# rm "$IMAGE_FILE.enc"
rm -rf "/tmp/t1"