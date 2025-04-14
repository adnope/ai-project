from fastapi import FastAPI, HTTPException, Request, Response
import random
import uvicorn
import sys
from pydantic import BaseModel
from typing import List
from fastapi.middleware.cors import CORSMiddleware
import logging

from solver_interface import call_solver, init_process

process = init_process()

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Setup logging to both file and console with immediate flush
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(message)s',
    handlers=[
        logging.FileHandler('connect4.log', mode='a'),
        logging.StreamHandler(sys.stdout)
    ]
)

def force_log(message):
    logging.info(message)
    # Force flush all handlers
    for handler in logging.getLogger().handlers:
        handler.flush()

@app.middleware("http")
async def log_request_response(request: Request, call_next):
    # Log the request
    body = await request.body()
    logging.info(f"Request: {request.method} {request.url} Body: {body.decode('utf-8')}")
    
    response = await call_next(request)
    
    # Collect and log the response body
    response_body = b""
    async for chunk in response.body_iterator:
         response_body += chunk
    logging.info(f"Response: {response.status_code} Body: {response_body.decode('utf-8')}\n")
    
    return Response(content=response_body, status_code=response.status_code, headers=response.headers)

class GameState(BaseModel):
    board: List[List[int]]
    current_player: int
    valid_moves: List[int]

class AIResponse(BaseModel):
    move: int

def is_valid_board(curr_board, target_board):
    """Check if current board could lead to target board"""
    for r in range(6):
        for c in range(7):
            if curr_board[r][c] != 0 and curr_board[r][c] != target_board[r][c]:
                return False
    return True

def is_valid_connect4_board(board):
    """Check if pieces are properly stacked due to gravity"""
    for col in range(7):
        for row in range(4, -1, -1):
            if board[row][col] != 0 and board[row + 1][col] == 0:
                return False  # Floating piece found
    return True

def dfs_find_sequence(target_board, curr_board, move_seq, curr_player, piece_count):
    """Find sequence of moves using DFS"""
    # Base case: if we've placed all the pieces in the target board
    if piece_count == 0:
        return curr_board == target_board

    # Try each column
    for col in range(7):
        # Find lowest empty row in this column
        row = 5
        while row >= 0 and curr_board[row][col] != 0:
            row -= 1

        # If column is not full
        if row >= 0:
            # Make the move only if it matches target board
            if target_board[row][col] == curr_player:
                curr_board[row][col] = curr_player
                move_seq.append(col + 1)
                piece_count -= 1

                # Recursively try next move with other player
                next_player = 2 if curr_player == 1 else 1
                if dfs_find_sequence(target_board, curr_board, move_seq, 
                                   next_player, piece_count):
                    return True

                # If this path didn't work, undo the move
                curr_board[row][col] = 0
                move_seq.pop()
                piece_count += 1

    return False

def board_to_move_sequence(board):
    """Convert a board state to sequence of moves"""
    # First, verify the board is valid Connect 4 board
    if not is_valid_connect4_board(board):
        return ""  # Invalid board configuration

    move_seq = []
    curr_board = [[0] * 7 for _ in range(6)]

    # Count total pieces
    piece_count = sum(cell != 0 for row in board for cell in row)
    
    # Try starting with player 1
    remaining = piece_count
    if dfs_find_sequence(board, curr_board, move_seq, 1, remaining):
        return "".join(str(move) for move in move_seq)
    
    # If player 1 starting didn't work, try player 2
    move_seq.clear()
    curr_board = [[0] * 7 for _ in range(6)]
    remaining = piece_count
    if dfs_find_sequence(board, curr_board, move_seq, 2, remaining):
        return "".join(str(move) for move in move_seq)

    return ""  # No valid sequence found

import sys

def print_immediate(*args):
    """Print immediately to console"""
    print(*args, flush=True)
    sys.stdout.flush()

@app.post("/api/connect4-move")
async def make_move(game_state: GameState) -> AIResponse:
    try:
        print_immediate("\n" + "="*50)
        print_immediate("NEW MOVE REQUEST")
        print_immediate("="*50)
        
        if not game_state.valid_moves:
            raise ValueError("Không có nước đi hợp lệ")
            
        print_immediate("Current board:")
        for row in game_state.board:
            print_immediate(row)
        
        sequence = board_to_move_sequence(game_state.board)
        print_immediate(f"\nMove sequence: {sequence}")

        if sequence:
            solver_move = call_solver(sequence, process)
            print_immediate(f"\nSolver suggests move: {solver_move}")
            if solver_move is not None:
                selected_move = solver_move - 1
                if selected_move in game_state.valid_moves:
                    print_immediate(f"Using solver move: {selected_move}")
                    return AIResponse(move=selected_move)
        
        selected_move = random.choice(game_state.valid_moves)
        print_immediate(f"Using random move: {selected_move}")
        return AIResponse(move=selected_move)
        
    except Exception as e:
        if game_state.valid_moves:
            return AIResponse(move=game_state.valid_moves[0])
        raise HTTPException(status_code=400, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8080)