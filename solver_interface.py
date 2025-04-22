import subprocess
import os
import sys
from typing import Optional
from board_tracker import BoardTracker

class SolverInterface:
    def __init__(self):
        print("[Solver] Initializing solver interface")
        self.process = None
        self.board_tracker = BoardTracker()
        self.last_solve_time = None
        self.solved_sequence = None

    def init_process(self):
        try:
            if self.process:
                print("[Solver] Closing existing solver process")
                self.process.terminate()
                self.process = None

            solver_path = os.path.join(os.path.dirname(__file__), "main")
            print(f"[Solver] Starting new solver process: {solver_path}")

            self.process = subprocess.Popen(
                [solver_path, "-f"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            return self.process
        except Exception as e:
            print(f"[Solver] Error: {str(e)}")
            return None

    def call_solver(self, board: list[list[int]], player: int) -> Optional[int]:
        try:
            print(f"[Solver] Call solver with player {player}")

            # Check for new game and reinit process
            piece_count = sum(cell != 0 for row in board for cell in row)
            if piece_count <= 1:
                print("[Solver] New game detected - resetting state and creating new solver process")
                self.board_tracker.reset()
                self.init_process()

            # First check for opponent move
            self.board_tracker.handle_new_request(board)

            # Print the sequence we're sending to solver
            sequence = self.board_tracker.get_sequence()

            # Reset timing and move info
            self.last_solve_time = None

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
            print(f"[Solver] Error: {str(e)}")
            return None

    def _call_solver_with_sequence(self, sequence: str) -> Optional[int]:
        try:
            self.process.stdin.write(f"{sequence}\n")
            self.process.stdin.flush()

            while True:
                line = self.process.stdout.readline().strip()
                
                if "moves" in line:
                    index = line.find("moves")
                    if index != -1:
                        self.solved_sequence = line[:index + len("moves")] # Get the moves part: 4444: 4 moves

                if "Time:" in line:
                    try:
                        self.last_solve_time = float(line.split("Time:")[1].split("ms")[0].strip())
                    except:
                        pass

                if "Best move: column" in line:
                    return int(line.split("Best move: column ")[-1].strip())

                if not line:
                    print("[Solver] No more output")
                    break

            return None

        except Exception as e:
            print(f"[Solver] Error: {str(e)}")
            return None

    def reset(self):
        """Reset everything - close solver and clear board"""
        print("[Solver] Full reset - closing solver and clearing board")
        if self.process:
            self.process.terminate()
            self.process = None
        self.board_tracker.reset()