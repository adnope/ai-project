import subprocess
import os
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
            solver_path = os.path.join(os.path.dirname(__file__), "../../bin/main")
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

    def call_solver(self, board: list[list[int]], player: int, is_new_game: bool) -> Optional[int]:
        try:
            print(f"[Solver] Call solver with player {player}")

            if is_new_game:
                print("[Solver] New game detected - resetting board tracker and sequence")
                self.reset()

            self.board_tracker.handle_new_request(board, is_new_game, player)

            sequence = self.board_tracker.get_sequence()

            move = self._call_solver_with_sequence(sequence)

            if move is not None:
                self.board_tracker.add_our_move(move - 1, player)  # Convert 1-based to 0-based
            else:
                print("[Solver] WARNING: No move returned from main process")

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
                        part = line[:index + len("moves")]
                        self.solved_sequence = part.replace(":", ",", 1)

                if "Time:" in line:
                    try:
                        self.last_solve_time = float(line.split("Time:")[1].split("ms")[0].strip())
                    except:
                        pass

                if "Best move: column" in line:
                    return int(line.split("Best move: column ")[-1].strip())

                if not line:
                    print("[Solver] No output detected from main")
                    break

            return None

        except Exception as e:
            print(f"[Solver] Error: {str(e)}")
            return None

    def reset(self):
        self.last_solve_time = None
        self.solved_sequence = None
        self.board_tracker.reset()