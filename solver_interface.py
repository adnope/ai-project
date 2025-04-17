import subprocess
import os
import logging
import sys
from typing import Optional
from board_tracker import BoardTracker

class SolverInterface:
    def __init__(self):
        print("[Solver] Initializing solver interface")
        self.process = self.init_process()
        self.board_tracker = BoardTracker()

    def init_process(self):
        try:
            solver_path = os.path.join(os.path.dirname(__file__), "main")
            print(f"[Solver] Starting solver process: {solver_path}")
            sys.stdout.flush()

            process = subprocess.Popen(
                [solver_path, "-f"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            print("[Solver] Solver process started successfully")
            return process
        except Exception as e:
            logging.error(f"Solver error: {str(e)}")
            print(f"[Solver] ERROR initializing solver: {str(e)}")
            return None

    def call_solver(self, board: list[list[int]], player: int) -> Optional[int]:
        try:
            print(f"[Solver] Call solver with player {player}")
            
            # Check for new game or first move
            piece_count = sum(cell != 0 for row in board for cell in row)
            if piece_count <= 1:
                print("[Solver] New game or first move detected, resetting")
                self.board_tracker.reset()
            
            
            # First check for opponent move
            self.board_tracker.handle_new_request(board)
            
            # Print the sequence we're sending to solver
            sequence = self.board_tracker.get_sequence()
            # print(f"[Solver] Calling solver with sequence: '{sequence}'")
            
            # Get solver move
            move = self._call_solver_with_sequence(sequence)
            
            # Add our move to sequence immediately if valid
            if move is not None:
                if (move == 9):
                    print("[Solver] Timed out.")
                    return None
                self.board_tracker.add_our_move(move - 1, player)  # Convert 1-based to 0-based
            else:
                print("[Solver] WARNING: Solver returned None for move")
                
            return move
            
        except Exception as e:
            logging.error(f"Solver interface error: {str(e)}")
            print(f"[Solver] ERROR in call_solver: {str(e)}")
            return None

    def _call_solver_with_sequence(self, sequence: str) -> Optional[int]:
        try:
            # print(f"[Solver] Writing to solver stdin: {sequence}")
            self.process.stdin.write(f"{sequence}\n")
            self.process.stdin.flush()

            move_found = False
            line_count = 0
            while True:
                line = self.process.stdout.readline()
                line_count += 1
                
                print(f"[Solver] Output {line_count}: {line.strip()}")

                if "Best move: column" in line:
                    move = int(line.split("Best move: column ")[-1].strip())
                    print(f"[Solver] Found best move: {move}")
                    move_found = True
                    return move
                
                # Exit 
                if line_count > 100:
                    print("[Solver] Too many lines read without finding a move")
                    break

            if not move_found:
                print("[Solver] WARNING: No move found in solver output")
            return None

        except Exception as e:
            logging.error(f"Solver error: {str(e)}")
            print(f"[Solver] ERROR in _call_solver_with_sequence: {str(e)}")
            return None

    def reset(self):
        """Reset the board tracker"""
        print("[Solver] Resetting solver interface")
        self.board_tracker.reset()