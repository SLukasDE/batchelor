#/bin/sh

# without authorization
#toolcontainer ./build/batchelor-head/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-head -S basic -s port 8080 -s threads 4

# assignes user user 'hans' the role 'execute' in namespace 'default' and assignes api-key 'AXBS5' the roles 'execute' and 'worker' in namespace 'default'
toolcontainer ./build/batchelor-head/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-head -S basic -s port 8080 -s threads 4 -B hans plain:wurst -U hans default execute -A AXBS5 default worker -A AXBS5 default execute
