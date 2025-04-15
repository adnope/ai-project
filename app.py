from fastapi import FastAPI, HTTPException, Request, Response
import random
import uvicorn
import sys
from pydantic import BaseModel
from typing import List
from fastapi.middleware.cors import CORSMiddleware
import logging

from solver_interface import SolverInterface


solver = SolverInterface()

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

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

def is_board_full(board: List[List[int]]) -> bool:
    """Check if board has no empty spaces"""
    return all(cell != 0 for row in board for cell in row)

def is_winning_board(board: List[List[int]]) -> bool:
    # Horizontal
    for row in range(6):
        for col in range(4):
            window = [board[row][col + i] for i in range(4)]
            if len(set(window)) == 1 and window[0] != 0:
                return True
    
    # Vertical
    for row in range(3):
        for col in range(7):
            window = [board[row + i][col] for i in range(4)]
            if len(set(window)) == 1 and window[0] != 0:
                return True
                
    # Diagonal right
    for row in range(3):
        for col in range(4):
            window = [board[row + i][col + i] for i in range(4)]
            if len(set(window)) == 1 and window[0] != 0:
                return True
                
    # Diagonal left
    for row in range(3):
        for col in range(3, 7):
            window = [board[row + i][col - i] for i in range(4)]
            if len(set(window)) == 1 and window[0] != 0:
                return True
    
    return False

def print_immediate(*args):
    print(*args, flush=True)
    sys.stdout.flush()

@app.post("/api/connect4-move")
async def make_move(game_state: GameState) -> AIResponse:
    try:
        print_immediate("\n" + "="*50)
        print_immediate("NEW MOVE REQUEST")
        print_immediate("="*50)
        
        if is_board_full(game_state.board):
            print_immediate("Board is full - Starting new game")
            solver.reset()
        elif is_winning_board(game_state.board):
            print_immediate("Game won - Starting new game")
            solver.reset()
        elif not game_state.valid_moves:
            print_immediate("No valid moves - Starting new game")
            solver.reset()
            raise ValueError("No valid moves available")
            
        print_immediate("Current board:")
        for row in game_state.board:
            print_immediate(row)
        
        print_immediate(f"Current player: {game_state.current_player}")
        print_immediate(f"Valid moves: {game_state.valid_moves}")
        
        solver_move = solver.call_solver(game_state.board, game_state.current_player)
        if solver_move is not None:
            selected_move = solver_move - 1  # Convert 1-based to 0-based
            print_immediate(f"Solver returned move: {solver_move} (0-indexed: {selected_move})")
            
            if selected_move in game_state.valid_moves:
                return AIResponse(move=selected_move)
            else:
                print_immediate(f"WARNING: Solver move {selected_move} not in valid moves {game_state.valid_moves}")
        else:
            print_immediate("Solver returned None")
        

        selected_move = random.choice(game_state.valid_moves)
        print_immediate(f"Using random move: {selected_move}")
        return AIResponse(move=selected_move)
        
    except Exception as e:
        print_immediate(f"ERROR in make_move: {str(e)}")
        if game_state.valid_moves:
            # Fallback to first valid move on error
            return AIResponse(move=game_state.valid_moves[0])
        raise HTTPException(status_code=400, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8080)