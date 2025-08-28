rm -rf simios_lib
mkdir simios_lib
cd simios_lib

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0\

  # -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \

cmake --build . --config Release -- -j8
# Pick the output lib (simulator .a)
