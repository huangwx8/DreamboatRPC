from flask import Flask, request, jsonify
import argparse
import os
import sys
import time
from threading import Thread


app = Flask(__name__)

service_map = {}

MAX_INACTIVITY_TIME = 30

MAX_OFFLINE_TIME = 30

def monitor_clients():
    while True:
        current_time = time.time()
        for client_id in list(service_map.keys()):
            if service_map[client_id]['status'] == 'ONLINE':
                if current_time - service_map[client_id]['alive_timestamp'] > MAX_INACTIVITY_TIME:
                    service_map[client_id]['status'] = 'OFFLINE'
                    print(f"{client_id} goes OFFLINE", file=sys.stderr)
            elif service_map[client_id]['status'] == 'OFFLINE':
                if current_time - service_map[client_id]['alive_timestamp'] > MAX_INACTIVITY_TIME + MAX_OFFLINE_TIME:
                    service_map.pop(client_id, None)
                    print(f"{client_id} is removed", file=sys.stderr)
        time.sleep(10)

# Start the monitoring thread
monitor_thread = Thread(target=monitor_clients, daemon=True)
monitor_thread.start()

@app.route('/discover', methods=['GET'])
def discover_endpoint():
    service_list = list(service_map.values())
    return jsonify(service_list)

@app.route('/register', methods=['POST'])
def register_endpoint():
    data = request.json
    client_id = f"{data['ip']}:{data['port']}"
    
    service_map[client_id] = data
    service_map[client_id]['alive_timestamp'] = time.time()
    service_map[client_id]['status'] = 'ONLINE'
    
    response = {"status": "success"}
    
    return jsonify(response)

@app.route('/heartbeat', methods=['POST'])
def heartbeat_endpoint():
    data = request.json
    client_id = f"{data['ip']}:{data['port']}"
    
    if not client_id in service_map.keys():
        return jsonify({"status": "error", "message": "not-registered."}), 400
    
    service_map[client_id]['alive_timestamp'] = time.time()
    service_map[client_id]['status'] = 'ONLINE'
    
    return jsonify({"status": "success"})

def parse_args():
    parser = argparse.ArgumentParser(description="Flask server with customizable IP and port.")
    parser.add_argument('-host', type=str, default='0.0.0.0', help='IP address to bind the server to')
    parser.add_argument('-port', type=int, default=5000, help='Port to run the server on')
    return parser.parse_args()

if __name__ == '__main__':
    os.environ['FLASK_ENV'] = 'production'
    args = parse_args()
    app.run(host=args.host, port=args.port, debug=False)
