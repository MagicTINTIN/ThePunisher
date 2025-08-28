rm -rf ios_lib
mkdir ios_lib
cd ios_lib

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0\

cmake --build . --config Release -- -j8