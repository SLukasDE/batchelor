#!/bin/sh

#pip install git+https://github.com/larsks/dockerize

rm -rf build
cmake -G "Unix Makefiles" -S . -B build
cmake --build ./build

dockerize -o ./tmp --no-build \
  -a  ./build/bin/batchelor-docker                                                                                       /bin/batchelor    \
  -a ../batchelor-head/build/batchelor-head/1.0.0/default/architecture/linux-gcc/link-executable/batchelor-head          /bin/batchelor-head    \
  -a ../batchelor-ui/build/batchelor-ui/1.0.0/default/architecture/linux-gcc/link-executable/batchelor-ui                /bin/batchelor-ui      \
  -a ../batchelor-control/build/batchelor-control/1.0.0/default/architecture/linux-gcc/link-executable/batchelor-control /bin/batchelor-control \
  -a ../batchelor-worker/build/batchelor-worker/1.0.0/default/architecture/linux-gcc/link-executable/batchelor-worker    /bin/batchelor-worker  \
  -e /bin/batchelor
mv tmp/Dockerfile Dockerfile
docker build -t batchelor:0.0.1-amd64 -f Dockerfile tmp
rm -rf tmp Dockerfile

docker image save batchelor:amd64 > batchelor-amd64.tar
scp lukas@arm64-server:./batchelor-arm64.tar .
docker image load batchelor-arm64.tar

docker tag batchelor:amd64 slukasde/batchelor:0.0.1-amd64
docker tag batchelor:arm64 slukasde/batchelor:0.0.1-arm64

docker push slukasde/batchelor:0.0.1-amd64
docker push slukasde/batchelor:0.0.1-arm64

docker manifest create slukasde/batchelor:0.0.1 \
  --amend slukasde/batchelor:0.0.1-amd64 \
  --amend slukasde/batchelor:0.0.1-arm64
docker manifest push slukasde/batchelor:0.0.1

docker manifest create slukasde/batchelor:latest \
  --amend slukasde/batchelor:0.0.1-amd64 \
  --amend slukasde/batchelor:0.0.1-arm64
docker manifest push slukasde/batchelor:latest

exit 0
echo
echo "Run batchelor:"
docker run --rm batchelor:latest
echo "<< End batchelor"
echo

#docker image save batchelor:latest > batchelor.tar

echo
echo "Show content of batchelor:"
docker create --name="tmp_X" batchelor:latest
docker export tmp_X | tar tv
docker rm tmp_X
echo "<< End content"
echo
sleep 1

#docker login
#docker tag batchelor:latest slukasde/batchelor:latest
#docker push slukasde/batchelor:latest
