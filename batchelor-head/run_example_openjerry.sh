#!/bin/sh

cp ~/workspace_esl/open-jerry/build/open-jerry/1.6.0/default/architecture/linux-gcc/link-executable/open-jerry .
toolcontainer ./open-jerry -l logger.xml jerry.xml
rm open-jerry
