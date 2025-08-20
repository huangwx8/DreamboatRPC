#/usr/bin/sh
source venv/bin/activate

python namesvr/run_cluster.py > log/namesvr.log 2>&1 &
PID1=$!

sleep 1
./build/Server -namesvr_ip=127.0.0.1 -namesvr_port=5566 -ip=127.0.0.1 -port=8888 -log=server.log &
PID2=$!

# Wait for a CTRL+C to continue
trap "echo 'Stopping servers...'; pgrep -f 'run_cluster.py' | xargs kill -9 > /dev/null 2>&1; pgrep -f 'run_node.py' | xargs kill -9 > /dev/null 2>&1; pgrep -f 'spawn_main' | xargs kill -9 > /dev/null 2>&1; exit" INT
echo "Servers running. Press CTRL+C to stop."
while true; do sleep 1; done
