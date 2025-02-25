from flask import Flask, request, jsonify
import argparse
import os

import json

app = Flask(__name__)

service_list = []

@app.route('/discover', methods=['GET'])
def discover_endpoint():
    return jsonify(service_list)

@app.route('/register', methods=['POST'])
def register_endpoint():
    data = request.json
    
    service_list.append(data)
    
    response = {"status": "success"}
    
    return jsonify(response)

def parse_args():
    parser = argparse.ArgumentParser(description="Flask server with customizable IP and port.")
    parser.add_argument('-host', type=str, default='0.0.0.0', help='IP address to bind the server to')
    parser.add_argument('-port', type=int, default=5000, help='Port to run the server on')
    return parser.parse_args()

if __name__ == '__main__':
    os.environ['FLASK_ENV'] = 'production'
    args = parse_args()
    app.run(host=args.host, port=args.port, debug=False)
