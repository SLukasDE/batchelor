#!/bin/sh

cp ~/workspace_esl/jerry/jerry-cli/build/jerry-cli/1.6.0/default/architecture/linux-gcc/link-executable/jerry .
toolcontainer ./jerry -l logger.xml jerry.xml

