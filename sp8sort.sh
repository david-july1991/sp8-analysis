#!/bin/sh
docker run -ti --rm -v HOST:DOCKER david9107/sp8-analysis sp8sort `realpath $*`
