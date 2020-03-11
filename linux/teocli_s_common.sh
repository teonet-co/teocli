#!/bin/bash
# for i in {1..37} 
for i in {1..10} 
do
    echo "Welcome $i times"
    ./teocli_s_common tcli_$i 127.0.0.1 7500 ps-server &
    # ./teocli_s_common tcli_$i 10.135.132.126 9010 teo-mm &
    usleep 150000
done 

# Default 
#
# sudo sysctl -w net.core.rmem_default=212992
# sudo sysctl -w net.core.rmem_max=212992

# Upgrade to 25 Mb
#
# sudo sysctl -w net.core.rmem_default=26214400
# sudo sysctl -w net.core.rmem_max=26214400
