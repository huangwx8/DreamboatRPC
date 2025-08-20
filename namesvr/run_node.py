#!/usr/bin/env python
import sys
import json
import time
from flask import Flask, request, jsonify
from raft import RaftNode
import threading

MAX_INACTIVITY_TIME = 30
MAX_OFFLINE_TIME = 30


def start_http_server(node, port):
    app = Flask(__name__)

    @app.route("/request", methods=["POST"])
    def client_request():
        data = request.get_json()
        if not data:
            return jsonify({"error": "No JSON payload"}), 400
        node.client_request(data)
        return jsonify({"status": "ok", "data": data})

    @app.route("/status", methods=["GET"])
    def status():
        entry = node.check_committed_entry()
        role = node.check_role()
        return jsonify({"committed_entry": entry, "role": role})

    @app.route('/discover', methods=['GET'])
    def discover_endpoint():
        callee = request.args.get('callee', None)  # Get 'callee' from the query parameters
        if callee is None:
            return jsonify({"status": "error", "message": "'callee' parameter is required."}), 400

        servers = node.check_committed_entry()
        app.logger.debug(f"discover, callee={callee}, servers={servers}")

        best_svr = None
        best_load = float('inf')  # Set an initial high value for comparison

        # Iterate over all registered services
        for _, svr in servers.items():
            if callee not in svr['services']:
                continue
            
            cpu_usage = svr.get("cpu_usage", 0)
            memory_usage = svr.get("memory_usage", 0)

            # Calculate the combined load: you could use a weighted sum, here we use a simple sum
            total_load = cpu_usage + memory_usage
            
            # If this service has a lower load, update the best service
            if total_load < best_load:
                best_load = total_load
                best_svr = svr
        
        # return it to the client
        if best_svr:
            return jsonify([
                {
                    'ip' : best_svr['ip'],
                    'port' : best_svr['port'],
                }
            ])

        return jsonify([])

    @app.route('/register', methods=['POST'])
    def register_endpoint():
        data = request.json
        ip = str(data['ip'])
        port = int(data['port'])
        service_name = str(data['service_name'])
        servers = node.check_committed_entry()

        app.logger.debug(f"register, data={data}")

        svr_id = f"{ip}:{port}"

        # Register the service under its node (ip:port)
        if svr_id not in servers:
            servers[svr_id] = {
                'ip': ip,
                'port': port,
                'services': [],
                'alive_timestamp': time.time(),
                'status': 'ONLINE',
                'cpu_usage': 0,
                'memory_usage': 0,
            }

        if service_name in servers[svr_id]['services']:
            return jsonify({"status": "error", "message": f"Duplicate service {service_name} on {ip}:{port}"}), 400

        servers[svr_id]['services'].append(service_name)
        servers[svr_id]['alive_timestamp'] = time.time()
        servers[svr_id]['status'] = 'ONLINE'
        node.client_request(servers)

        response = {"status": "success"}
        return jsonify(response)

    @app.route('/heartbeat', methods=['POST'])
    def heartbeat_endpoint():
        data = request.json
        ip = str(data['ip'])
        port = int(data['port'])
        servers = node.check_committed_entry()

        app.logger.debug(f"heartbeat, data={data}")

        svr_id = f"{ip}:{port}"

        if svr_id not in servers:
            return jsonify({"status": "error", "message": "not-registered."}), 400

        servers[svr_id]['alive_timestamp'] = time.time()
        servers[svr_id]['status'] = 'ONLINE'
        servers[svr_id]['cpu_usage'] = data.get('cpu_usage', 0)
        servers[svr_id]['memory_usage'] = data.get('memory_usage', 0)
        node.client_request(servers)

        return jsonify({"status": "success"})

    def monitor_clients():
        while True:
            if node.check_role() != 'leader':
                continue
            current_time = time.time()
            servers = node.check_committed_entry()
            print(f"monitor_clients, servers={servers}")
            for svr_id in list(servers.keys()):
                # If a node goes inactive, mark all its services as offline
                if servers[svr_id]['status'] == 'ONLINE':
                    if current_time - servers[svr_id]['alive_timestamp'] > MAX_INACTIVITY_TIME:
                        servers[svr_id]['status'] = 'OFFLINE'
                        app.logger.debug(f"{svr_id} goes OFFLINE", file=sys.stderr)
                elif servers[svr_id]['status'] == 'OFFLINE':
                    if current_time - servers[svr_id]['alive_timestamp'] > MAX_INACTIVITY_TIME + MAX_OFFLINE_TIME:
                        servers.pop(svr_id, None)
                        app.logger.debug(f"{svr_id} is removed", file=sys.stderr)
            node.client_request(servers)
            time.sleep(10)

    # Start the monitoring thread
    monitor_thread = threading.Thread(target=monitor_clients, daemon=True)
    monitor_thread.start()

    # Run Flask in a separate thread so it doesn't block the Raft node
    threading.Thread(
        target=lambda: app.run(host="0.0.0.0", port=port, debug=False, use_reloader=False)
    ).start()


def main(config_path):
    with open(config_path, "r") as f:
        config = json.load(f)

    self_name = config["self"]
    comm_dict = config["nodes"]

    node = RaftNode(comm_dict, self_name)
    node.start()

    raft_port = int(comm_dict[self_name]["port"])
    http_port = raft_port + 1
    start_http_server(node, http_port)

    print(f"[{self_name}] Raft node started on {comm_dict[self_name]['ip']}:{raft_port}")
    print(f"[{self_name}] HTTP server listening on {comm_dict[self_name]['ip']}:{http_port}")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print(f"[{self_name}] stopping...")
        node.stop()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: run_node.py <config.json>")
        sys.exit(1)
    main(sys.argv[1])
