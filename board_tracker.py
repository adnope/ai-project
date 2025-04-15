class BoardTracker:
    def __init__(self):
        self.last_board = [[0] * 7 for _ in range(6)]
        self.sequence = ""
        print("[Tracker] Initialized new board tracker")
    
    def detect_opponent_move(self, new_board) -> int:
        changes_found = 0
        opponent_col = -1
        
        for col in range(7):
            for row in range(5, -1, -1):
                if (new_board[row][col] != 0 and 
                    new_board[row][col] != self.last_board[row][col]):
                    changes_found += 1
                    opponent_col = col
                    print(f"[Tracker] Detected change at row {row}, col {col}: {self.last_board[row][col]} -> {new_board[row][col]}")
        
        # exactly one change for an opponent move
        if changes_found == 0:
            print("[Tracker] Warning: No changes detected between boards")
            return -1
        elif changes_found > 1:
            print(f"[Tracker] Warning: {changes_found} changes detected between boards, expected 1")
            print("[Tracker] Last board:")
            for row in self.last_board:
                print(row)
            print("[Tracker] New board:")
            for row in new_board:
                print(row)
            
        return opponent_col
    
    def handle_new_request(self, board) -> None:
        # Check if this is a completely new game
        is_empty_board = all(cell == 0 for row in board for cell in row)
        if is_empty_board:
            print("[Tracker] Empty board - starting new game")
            self.reset()
            return
            
        # Check if this is first move of a new game
        piece_count = sum(cell != 0 for row in board for cell in row)
        if piece_count == 1:
            print("[Tracker] First move detected - starting new sequence")
            self.reset()
            # Find the first move
            for col in range(7):
                for row in range(5, -1, -1):
                    if board[row][col] != 0:
                        print(f"[Tracker] Found first move at column {col + 1}")
                        self.sequence = str(col + 1)
                        self.last_board = [row[:] for row in board]
                        return
        
        # Normal move detection for ongoing game
        opp_col = self.detect_opponent_move(board)
        if opp_col != -1:
            print(f"[Tracker] Detected opponent move in column {opp_col + 1}")
            self.sequence += str(opp_col + 1)
        
        # Update board state
        self.last_board = [row[:] for row in board]
        print(f"[Tracker] Current sequence: {self.sequence}")
    
    def add_our_move(self, col: int, player: int):
        self.sequence += str(col + 1)
        
        move_placed = False
        for row in range(5, -1, -1):
            if self.last_board[row][col] == 0:
                self.last_board[row][col] = player
                print(f"[Tracker] Added our move in column {col + 1} at row {row}")
                move_placed = True
                break
        
        if not move_placed:
            print(f"[Tracker] WARNING: Failed to place our move in column {col + 1}, column appears full")
                
        print(f"[Tracker] Current sequence: {self.sequence}")
    
    def get_sequence(self) -> str:
        return self.sequence
    
    def reset(self):
        print("[Tracker] Resetting board tracker")
        self.last_board = [[0] * 7 for _ in range(6)]
        self.sequence = ""