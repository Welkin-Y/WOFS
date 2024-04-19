#! /bin/bash
make clean > /dev/null 2>&1
make > /dev/null 2>&1
make test  > /dev/null 2>&1
cd build
mkdir d0 > /dev/null 2>&1
cd d0
echo "ldkfjslkfjs;" > f0
mkdir d1  > /dev/null 2>&1
cd d1
echo "flgkdsjfkldsjglkdsjgl;k" > f1
echo 1111111111111111111111111111111111111111111111111111111111111111111111 > f2
mkdir d2  > /dev/null 2>&1
cd d2
echo "abcdefghigiajg;iajfjga;fjha;jha;" > f3
echo > f4
cd ../.. && mkdir d4  > /dev/null 2>&1
cd d4 
echo "12345678910" > f6
cd ../../
echo 123456 | ./wofs gen -e -c d0 testImage
mkdir mountpoint > /dev/null 2>&1
echo 123456 | ./wofs mount -c -e testImage.enc mountpoint
echo "compare file strcture"
if diff -r d0 mountpoint; then
    echo "Data test completed successfully: No differences found."
    umount mountpoint
    exit 0
else
    echo "Test failed: Differences found."
    umount mountpoint
    exit 1
fi
exit 1

