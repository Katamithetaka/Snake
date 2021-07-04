cmake -E make_directory build

cd build

cmake -DCMAKE_C_FLAGS="-mwindows" ..
cmake --build . --config Release

pause