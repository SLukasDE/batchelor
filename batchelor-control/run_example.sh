#!/bin/sh

#toolcontainer ./build/batchelor-control/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-control send-event -C basic -s url http://localhost:8080 -e batch-1 -W 3
toolcontainer ./build/batchelor-control/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-control send-event -C basic -s url http://localhost:8080 -s username hans -s password wurst -e batch-1 -W 3
