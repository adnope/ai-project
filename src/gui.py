import pygame.gfxdraw
import pygame
import time
import src.game as game

def hex_to_rgb(hex_color):
    hex_color = hex_color.strip("#")
    return tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))

BLUE = hex_to_rgb("#C3C7F4")
BLACK = hex_to_rgb("#EBEDFD")
DEEP_BLUE = hex_to_rgb("#FF8C00")
ORANGE = hex_to_rgb("#0077B6")
WHITE = hex_to_rgb("#9499D8")


ROWS = 6
COLS = 7
SQUARE_SIZE = 100
RADIUS = int(SQUARE_SIZE/2 - 5)

width = (COLS + 3) * SQUARE_SIZE
height = (ROWS + 1) * SQUARE_SIZE
size = (width, height)

clock = pygame.time.Clock()

screen = pygame.display.set_mode(size)
screen.fill(BLACK)

pygame.init()

# font
font = pygame.font.Font(None, 75)

# button play
button_play = pygame.Rect(10, 10, 150, 100)

def draw_board(board):
    draw_button(screen, BLUE, button_play, 20, "Start")

    # pygame.draw.rect(screen, BLACK, (SQUARE_SIZE*3, 0, SQUARE_SIZE * COLS, SQUARE_SIZE))
    
    for row in range(ROWS):
        for col in range(COLS):

            pygame.draw.rect(screen, BLUE, (col*SQUARE_SIZE + 3 * SQUARE_SIZE, row*SQUARE_SIZE + SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE))

            if board[row][col] == 0:
                draw_circle(screen, col*SQUARE_SIZE + 7 *SQUARE_SIZE/2, row*SQUARE_SIZE + 3*SQUARE_SIZE/2, RADIUS, BLACK)
            elif board[row][col] == 1:
                draw_circle(screen, col*SQUARE_SIZE + 7 *SQUARE_SIZE/2, row*SQUARE_SIZE + 3*SQUARE_SIZE/2, RADIUS, DEEP_BLUE)
            elif board[row][col] == 2:
                draw_circle(screen, col*SQUARE_SIZE + 7 *SQUARE_SIZE/2, row*SQUARE_SIZE + 3*SQUARE_SIZE/2, RADIUS, ORANGE)
            pygame.display.update()


def draw_circle (surface, x, y, radius, color):
    pygame.gfxdraw.aacircle(surface, int(x), int(y), radius, color)
    pygame.gfxdraw.filled_circle(surface, int(x), int(y), radius, color)


def draw(surface, turn, posx):
    pygame.draw.rect(surface, BLACK, (SQUARE_SIZE*3, 0, SQUARE_SIZE * COLS, SQUARE_SIZE))
    if (turn == 1):
        draw_circle(surface, posx, SQUARE_SIZE/2, RADIUS, ORANGE)
    else:
        draw_circle(surface, posx, SQUARE_SIZE/2, RADIUS, DEEP_BLUE)

def draw_win_screen(board, piece, color):
    pygame.draw.rect(board, BLACK, (SQUARE_SIZE*3, 0, SQUARE_SIZE * COLS, SQUARE_SIZE))
    label = font.render("Player " + str(piece) + " wins!", 1, color)
    screen.blit(label, (480, 30))

def draw_button (screen, color, button, boder_radius, text):
    pygame.draw.rect(screen, BLUE, button_play, border_radius=10)

    text_surface = font.render(text, True, WHITE)
    text_rect = text_surface.get_rect(center=button_play.center)
    screen.blit(text_surface, text_rect)

# def animate_drop_piece(board, row, col, piece):
#     if row is None:
#         return
    
#     color = DEEP_BLUE if piece == 1 else ORANGE
#     x = col * SQUARE_SIZE + 7 * SQUARE_SIZE // 2
#     y_start = SQUARE_SIZE // 2
#     y_end = (row + 1) * SQUARE_SIZE + SQUARE_SIZE // 2

#     y = y_start
#     v = 5
#     a = 1

#     y = y_start
#     velocity = 5  # Tốc độ rơi ban đầu
#     gravity = 0.5  # Tăng tốc dần khi rơi
#     max_speed = 20  # Giới hạn tốc độ tối đa

#     while y < y_end:
#         # screen.fill(BLACK)  # Xóa màn hình (hoặc chỉ vẽ lại phần cờ)
#         draw_board(board)  # Vẽ lại bàn cờ

#         pygame.draw.circle(screen, color, (x, int(y)), RADIUS)  # Vẽ quân cờ
#         pygame.display.flip()  # Cập nhật toàn bộ màn hình

#         velocity = min(velocity + gravity, max_speed)  # Tăng tốc rơi dần
#         y += velocity  # Cập nhật vị trí quân cờ

#         clock.tick(60)  # Đảm bảo game chạy mượt với 60 FPSn

#     board[row][col] = piece  # Cập nhật trạng thái bảng
#     return True
