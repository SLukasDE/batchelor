#/bin/sh

#toolcontainer ./build/batchelor-head/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-head -S basic -s port 8080 -s threads 4
toolcontainer ./build/batchelor-head/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-head -S basic -s port 8080 -s threads 4 -U hans g1 -B hans wurst -G g1 default worker -G g1 default execute
