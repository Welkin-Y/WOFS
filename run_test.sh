#! /bin/bash
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
./test
# rm -r d0