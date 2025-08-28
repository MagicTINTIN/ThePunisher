openmp-mobile/openmp.sh

cd libfun

./mac_buildlib.sh reset
./ios_buildlib.sh
./simios_buildlib.sh

# rm -rf lib/
# mkdir -p lib
cd ../

rm -rf libfun.xcframework

xcodebuild -create-xcframework \
  -library libfun/mac_lib/libfun.a \
  -library libfun/ios_lib/libfun.a \
  -library libfun/simios_lib/libfun.a \
  -output libfun.xcframework
