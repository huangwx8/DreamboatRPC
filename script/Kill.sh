#/bin/sh
pgrep -f 'namesvr.py' | xargs kill -9 > /dev/null 2>&1
pgrep -f 'build/Server' | xargs kill -9 > /dev/null 2>&1
pgrep -f 'build/Client' | xargs kill -9 > /dev/null 2>&1
