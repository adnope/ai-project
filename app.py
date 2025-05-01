import random
import uvicorn
import sys
import time
from fastapi import FastAPI, HTTPException, Request, Response
from pydantic import BaseModel
from typing import List
from fastapi.middleware.cors import CORSMiddleware
from solver_interface import SolverInterface

solver = SolverInterface()
solver.process = solver.init_process()
solver.call_solver([[0]], 1, 1)

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

def print_immediate(*args):
    print(*args, flush=True)
    sys.stdout.flush()

@app.get("/api/test")
async def health_check():
    return {"status": "ok", "message": "Server is running"}

@app.middleware("http")
async def log_request_response(request: Request, call_next):
    # Log the request
    body = await request.body()
    print_immediate(f"[App] Request: {request.method} {request.url} Body: {body.decode('utf-8')}")

    response = await call_next(request)

    # Collect and log the response body
    response_body = b""
    async for chunk in response.body_iterator:
         response_body += chunk
    print_immediate(f"[App] Response: {response.status_code} Body: {response_body.decode('utf-8')}\n")

    return Response(content=response_body, status_code=response.status_code, headers=response.headers)

class GameState(BaseModel):
    board: List[List[int]]
    current_player: int
    valid_moves: List[int]
    is_new_game: bool

class AIResponse(BaseModel):
    move: int

@app.post("/api/connect4-move")
async def make_move(game_state: GameState) -> AIResponse:
    api_time = time.perf_counter()
    try:
        print_immediate("\n" + "="*20)
        print_immediate("NEW MOVE REQUEST")
        print_immediate("="*20)

        print_immediate("[App] Current board:")
        for row in game_state.board:
            print_immediate(row)

        print_immediate(f"[App] Current player: {game_state.current_player}")
        print_immediate(f"[App] Valid moves: {game_state.valid_moves}")

        solver_move = solver.call_solver(game_state.board, game_state.current_player, game_state.is_new_game)
        if solver_move is not None:
            selected_move = solver_move - 1  # Convert 1-based to 0-based

            # Only print the best move and time
            if solver.last_solve_time is not None:
                print_immediate(f"[App] Solved sequence: {solver.solved_sequence}, Best move: {solver_move}, Time: {solver.last_solve_time} ms")
                print_immediate(f"[App] API time: {(time.perf_counter() - api_time) * 1000} ms")
                print_immediate(f"[Tracker] Current sequence: {solver.board_tracker.get_sequence()}")
            else:
                print_immediate(f"[App] Cannot find solving time")

            if selected_move in game_state.valid_moves:
                return AIResponse(move=selected_move)
            else:
                print_immediate(f"[App] WARNING: Solver move {selected_move} not in valid moves {game_state.valid_moves}")
        else:
            print_immediate("[App] Solver didn't return any move")
            selected_move = random.choice(game_state.valid_moves)
            print_immediate(f"[App] Using random move: {selected_move}")
            return AIResponse(move=selected_move)

    except Exception as e:
        print_immediate(f"[App] ERROR in make_move: {str(e)}")
        if game_state.valid_moves:
            return AIResponse(move=game_state.valid_moves[0])
        raise HTTPException(status_code=400, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8112)