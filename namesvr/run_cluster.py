#!/usr/bin/env python
import subprocess
import sys
import os
import time
import threading

import logging

CONFIGS = ["config/node0.json", "config/node1.json", "config/node2.json"]

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

def stream_output(proc, prefix):
    for line in proc.stdout:
        logging.info(f"[{prefix}] {line.strip()}")

def main():
    procs = []
    try:
        for cfg in CONFIGS:
            p = subprocess.Popen(
                [sys.executable, "run_node.py", cfg],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                cwd=os.path.dirname(os.path.abspath(__file__))
            )
            procs.append(p)
            logging.info(f"Started {cfg} (pid={p.pid})")
            # Launch a thread to forward logs
            threading.Thread(target=stream_output, args=(p, cfg), daemon=True).start()

        logging.info("Cluster running. Press Ctrl+C to stop.")

        while True:
            time.sleep(1)

    except KeyboardInterrupt:
        logging.info("Stopping cluster...")
    finally:
        for p in procs:
            if p.poll() is None:
                p.terminate()
        time.sleep(1)
        for p in procs:
            if p.poll() is None:
                p.kill()
        logging.info("All nodes stopped.")

if __name__ == "__main__":
    main()
