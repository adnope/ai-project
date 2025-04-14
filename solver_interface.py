import subprocess
import os
import logging
import sys
from typing import Optional

def init_process():
    try:
        solver_path = os.path.join(os.path.dirname(__file__), "main")
        sys.stdout.flush()

        process = subprocess.Popen(
            [solver_path, "-f"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        return process
    except Exception as e:
        logging.error(f"Solver error: {str(e)}")
    
def call_solver(sequence: str, process) -> Optional[int]:
    """Send a sequence to the persistent solver process and read the response."""
    try:
        # Send input
        process.stdin.write(f"{sequence}\n")
        process.stdin.flush()

        # Read output line-by-line until you find the move
        while True:
            line = process.stdout.readline()
            if not line:
                break  # EOF or broken pipe

            print(f"[Solver] {line.strip()}", flush=True)

            if "Best move: column" in line:
                move = int(line.split("Best move: column ")[-1].strip())
                logging.info(f"Found best move: {move}")
                return move

        return None

    except Exception as e:
        logging.error(f"Solver error: {str(e)}")
        return None