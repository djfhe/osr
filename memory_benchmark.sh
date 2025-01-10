#valgrind --tool=massif --stacks=yes --massif-out-file=massif-memory-benchmark-s-car ./cmake-build-debug/osr-memory-benchmark-s-car -d ./osr-data/osr-hessen -n 100 -r 1200
#valgrind --tool=massif --stacks=yes --massif-out-file=massif-memory-benchmark-g-car ./cmake-build-debug/osr-memory-benchmark-g-car -d ./osr-data/osr-hessen -n 100 -r 1200
valgrind --tool=massif --massif-out-file=massif-memory-benchmark-s-car-parking ./cmake-build-debug/osr-memory-benchmark-s-car-parking -d ./osr-data/osr-hessen -n 100 -r 1200
valgrind --tool=massif --massif-out-file=massif-memory-benchmark-g-car-parking ./cmake-build-debug/osr-memory-benchmark-g-car-parking -d ./osr-data/osr-hessen -n 100 -r 1200

