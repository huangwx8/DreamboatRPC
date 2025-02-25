#/bin/sh
pgrep -f 'namesvr' | xargs kill -9 > /dev/null 2>&1
pgrep -f 'Server' | xargs kill -9 > /dev/null 2>&1
