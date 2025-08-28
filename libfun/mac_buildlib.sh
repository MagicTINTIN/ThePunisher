if [ $# == 1 ] && [ $1 == "reset" ]; then
    rm -rf mac_lib/ 2> /dev/null
fi

mkdir -p mac_lib
cd mac_lib
cmake ..
make
cd ../