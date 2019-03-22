./build-asura.sh asura
./build-gpu-2.sh reduce

./asura $1
nLines=`wc -l tmp-asura | cut -f 1 `
./reduce tmp-asura $nLines
