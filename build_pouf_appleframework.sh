openmp-mobile/openmp.sh

cd libpouf

./mac_buildlib.sh reset
./ios_buildlib.sh
./simios_buildlib.sh

# rm -rf lib/
# mkdir -p lib
cd ../

rm -rf libpouf.xcframework

xcodebuild -create-xcframework \
  -library libpouf/mac_lib/libpouf.a \
  -library libpouf/ios_lib/libpouf.a \
  -library libpouf/simios_lib/libpouf.a \
  -output libpouf.xcframework
