#!/bin/sh

#toolcontainer ./build/batchelor-worker/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-worker -C basic -s url http://localhost:8080 -e batch-1 exec -s cmd /bin/sleep  -s args 10 -s args-flag fixed -s metric x86 -s cd /tmp
toolcontainer ./build/batchelor-worker/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-worker -C basic -s url http://localhost:8080 -s username hans -s password wurst -e batch-1 exec -s cmd /bin/sleep  -s args 10 -s args-flag fixed -s metric x86 -s cd /tmp
