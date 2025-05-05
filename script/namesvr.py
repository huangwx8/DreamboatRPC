from flask import Flask, request, jsonify
import logging
import argparse
import os
import sys
import time
from threading import Thread

app = Flask(__name__)

node_map = {}

MAX_INACTIVITY_TIME = 30
MAX_OFFLINE_TIME = 30


def monitor_clients():
    while True:
        current_time = time.time()
        for node_id in list(node_map.keys()):
            # If a node goes inactive, mark all its services as offline
            if node_map[node_id]['status'] == 'ONLINE':
                if current_time - node_map[node_id]['alive_timestamp'] > MAX_INACTIVITY_TIME:
                    node_map[node_id]['status'] = 'OFFLINE'
                    print(f"{node_id} goes OFFLINE", file=sys.stderr)
            elif node_map[node_id]['status'] == 'OFFLINE':
                if current_time - node_map[node_id]['alive_timestamp'] > MAX_INACTIVITY_TIME + MAX_OFFLINE_TIME:
                    node_map.pop(node_id, None)
                    print(f"{node_id} is removed", file=sys.stderr)
        time.sleep(10)

# Start the monitoring thread
monitor_thread = Thread(target=monitor_clients, daemon=True)
monitor_thread.start()

@app.route('/discover', methods=['GET'])
def discover_endpoint():
    callee = request.args.get('callee', None)  # Get 'callee' from the query parameters
    if callee is None:
        return jsonify({"status": "error", "message": "'callee' parameter is required."}), 400

    app.logger.debug(f"discover, callee={callee}, node_map={node_map}")

    best_node = None
    best_load = float('inf')  # Set an initial high value for comparison

    # Iterate over all registered services
    for _, node in node_map.items():
        if callee not in node['services']:
            continue
        
        cpu_usage = node.get("cpu_usage", 0)
        memory_usage = node.get("memory_usage", 0)

        # Calculate the combined load: you could use a weighted sum, here we use a simple sum
        total_load = cpu_usage + memory_usage
        
        # If this service has a lower load, update the best service
        if total_load < best_load:
            best_load = total_load
            best_node = node
    
    # return it to the client
    if best_node:
        return jsonify([
            {
                'ip' : best_node['ip'],
                'port' : best_node['port'],
            }
        ])

    return jsonify([])

@app.route('/register', methods=['POST'])
def register_endpoint():
    data = request.json
    ip = data['ip']
    port = data['port']
    service_name = data['service_name']

    app.logger.debug(f"register, data={data}")

    node_id = f"{ip}:{port}"

    # Register the service under its node (ip:port)
    if node_id not in node_map:
        node_map[node_id] = {
            'ip' : ip,
            'port' : port,
            'services': set(),
            'alive_timestamp': time.time(),
            'status': 'ONLINE',
            'cpu_usage' : 0,
            'memory_usage' : 0,
        }

    if service_name in node_map[node_id]['services']:
        return jsonify({"status": "error", "message": f"Duplicate service {service_name} on {ip}:{port}"}), 400

    node_map[node_id]['services'].add(service_name)
    node_map[node_id]['alive_timestamp'] = time.time()
    node_map[node_id]['status'] = 'ONLINE'
    
    response = {"status": "success"}
    return jsonify(response)


@app.route('/heartbeat', methods=['POST'])
def heartbeat_endpoint():
    data = request.json
    ip = data['ip']
    port = data['port']

    app.logger.debug(f"heartbeat, data={data}")

    node_id = f"{ip}:{port}"
    
    if node_id not in node_map:
        return jsonify({"status": "error", "message": "not-registered."}), 400

    node_map[node_id]['alive_timestamp'] = time.time()
    node_map[node_id]['status'] = 'ONLINE'
    node_map[node_id]['cpu_usage'] = data.get('cpu_usage', 0)
    node_map[node_id]['memory_usage'] = data.get('memory_usage', 0)

    return jsonify({"status": "success"})

def parse_args():
    parser = argparse.ArgumentParser(description="Flask server with customizable IP and port.")
    parser.add_argument('-host', type=str, default='0.0.0.0', help='IP address to bind the server to')
    parser.add_argument('-port', type=int, default=5000, help='Port to run the server on')
    return parser.parse_args()


if __name__ == '__main__':
    os.environ['FLASK_ENV'] = 'development'
    app.logger.setLevel(logging.DEBUG)
    args = parse_args()
    app.run(host=args.host, port=args.port, debug=False)
