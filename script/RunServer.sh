#/usr/bin/sh
python3 ./script/namesvr.py -host=127.0.0.1 -port=5000 > /dev/null 2>&1 &
sleep 1
./build/Server -namesvr_ip=127.0.0.1 -namesvr_port=5000 -ip=127.0.0.1 -port=8888 -log=server.log &
