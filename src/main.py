import math
import sys
import pygame
import src.gui as gui
import src.game as game

board = game.create_board()
print(board)
game_over = False
turn = 0
gui.draw_board(board)


while not game_over:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()

        if event.type == pygame.MOUSEMOTION:
            posx = event.pos[0]
            if (posx >= 7 * gui.SQUARE_SIZE / 2):
                gui.draw(gui.screen, turn, posx)
                    
        pygame.display.update()

        if event.type == pygame.MOUSEBUTTONDOWN:
            hover_color = gui.ORANGE if turn == 1 else gui.DEEP_BLUE
            pos = event.pos[0]

            if gui.button_play.collidepoint(event.pos):
                board = game.create_board()  # Reset game
                game_over = False
                turn = 0
                gui.draw_board(board)
            else:
                turn, game_over = game.process_turn(board, turn, pos)
                print(board)
                gui.draw_board(board)
        
    if game_over:
        pygame.time.delay(2000)