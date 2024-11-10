#!/bin/sh

toolcontainer ./build/batchelor-ui/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-ui -S basic -s port 9000 -s https false -C basic -s url http://localhost:8080 -s username hans -s password wurst -U hans default execute -B hans plain:wurst

