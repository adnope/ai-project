import numpy as np
import math
import src.gui as gui

# create board 
def create_board():
    return np.zeros((gui.ROWS, gui.COLS))

# check valid move
def is_valid_location(board, col):
    return board[0][col] == 0

#
def get_next_open_row(board, col):
    for row in range(gui.ROWS-1, -1, -1):
        if(board[row][col] == 0):
            return row
        
    return None

def drop_piece(board, row, col, piece):
    board[row][col] = piece

def check_win(board, piece):

    for row in range(gui.ROWS):
        for col in range(gui.COLS - 3):
            if all(board[row][col + i] == piece for i in range(4)):
                return True

    for row in range(gui.ROWS - 3):  
        for col in range(gui.COLS):
            if all(board[row + i][col] == piece for i in range(4)):
                return True
            
    for row in range(gui.ROWS - 3):          
        for col in range(gui.COLS - 3):
            if all(board[row + i][col + i] == piece for i in range(4)):
                return True
            
    for row in range(gui.ROWS - 3):
        for col in range(3, gui.COLS):
            if all(board[row + i][col - i] == piece for i in range(4)):
                return True
            
def process_turn(board, turn, pos):
    # pos = event.pos[0]
    
    if turn == 0:
        col = math.floor(pos / gui.SQUARE_SIZE - 3)
        # draw_board(board)

        if is_valid_location(board, col):
            row = get_next_open_row(board, col)
            drop_piece(board, row, col, 1)
            # gui.animate_drop_piece(board, row, col, 1)
            turn += 1
            turn = turn % 2
        if check_win(board, 1):
            gui.draw_win_screen(gui.screen, 1, gui.DEEP_BLUE)
            print("PLAYER 1 Wins!!!! Congrats!!!")
            return turn, True

    else:
        col = math.floor(pos / gui.SQUARE_SIZE - 3)

        if is_valid_location(board, col):
            row = get_next_open_row(board, col)
            drop_piece(board, row, col, 2)
            turn += 1
            turn = turn % 2
        
        if check_win(board, 2):
            gui.draw_win_screen(gui.screen, 2, gui.ORANGE)
            print("PLAYER 2 Wins!!!! Congrats!!!")
            return turn, True
        
    return turn, False


