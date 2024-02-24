#/bin/sh

# without authorization
#toolcontainer ./build/batchelor-head/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-head -S basic -s port 8080 -s threads 4

# assignes user 'hans' the role 'execute' in namespace 'default' and user 'worker' to roles 'execute' and 'worker' in namespace 'default'. Make user 'hans' usable by basic-auth and add an API key for user 'worker'
toolcontainer ./build/batchelor-head/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-head -S basic -s port 8080 -s threads 4 -U hans default execute -U worker default execute -U worker default worker -B hans plain:wurst -A worker plain:AXBS5
