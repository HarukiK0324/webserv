#!/bin/bash

URL="http://localhost:8081/"
echo "starting stress test on $URL"
while true; do
	curl -s $URL > /dev/null
done &
