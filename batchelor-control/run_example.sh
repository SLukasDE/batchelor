#!/bin/sh

# without authorization
#toolcontainer ./build/batchelor-control/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-control send-event -C basic -s url http://localhost:8080 -e batch-1 -W 3

# with basic authorization
toolcontainer ./build/batchelor-control/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-control send-event -C basic -s url http://localhost:8080 -s username hans -s password wurst -e batch-1 -W 3 -c true

# with bearer authorization
#toolcontainer ./build/batchelor-control/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-control send-event -C basic -s url http://localhost:8080 -s api-key AXBS5 -e batch-1 -W 3
