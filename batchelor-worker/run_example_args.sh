#!/bin/sh

# without authorization
#./build/batchelor-worker/1.0.0/default/architecture/linux-gcc/link-executable/batchelor-worker -m CLOUD_ID GCP -C basic -s url http://localhost:8080 -e batch-1 exec -s cmd /bin/sleep -s args 10 -s args-flag fixed -s cd /tmp

# with basic-authorization
#./build/batchelor-worker/1.0.0/default/architecture/linux-gcc/link-executable/batchelor-worker -m CLOUD_ID GCP -C basic -s url http://localhost:8080 -s username hans -s password wurst -e batch-1 exec -s cmd /bin/sleep -s args 10 -s args-flag fixed -s cd /tmp

# with bearer authentication
./build/batchelor-worker/1.0.0/default/architecture/linux-gcc/link-executable/batchelor-worker -m CLOUD_ID GCP -C basic -s url http://localhost:8080 -s api-key AXBS5 -e batch-1 exec -s cmd /bin/sleep -s args 10 -s args-flag fixed -s cd /tmp
