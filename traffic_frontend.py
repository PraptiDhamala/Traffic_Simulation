import pygame
import subprocess
import json
import threading
import queue
import math

pygame.init()
WIDTH, HEIGHT = 1100, 800  
WIN = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("SmartCity AI Traffic Controller")


ASPHALT = (30, 30, 35)
GRASS = (45, 120, 45)
DASHBOARD_BG = (20, 20, 25)
WHITE = (240, 240, 240)
RED, GREEN, YELLOW, BLUE = (255, 60, 60), (60, 255, 100), (255, 210, 50), (50, 160, 255)
GOLD = (212, 175, 55)

FONT_S = pygame.font.SysFont("Verdana", 14)
FONT_M = pygame.font.SysFont("Verdana", 18, bold=True)
FONT_L = pygame.font.SysFont("Verdana", 32, bold=True)

output_queue = queue.Queue()
PAUSED = False
RAIN = False

def run_c_program():
    proc = subprocess.Popen("./traffic", stdout=subprocess.PIPE, text=True)
    while True:
        line = proc.stdout.readline()
        if not line: break
        try:
            data = json.loads(line.strip())
            if not PAUSED: output_queue.put(data)
        except: continue

threading.Thread(target=run_c_program, daemon=True).start()

def draw_car(x, y, color, vertical=True, is_emergency=False):
    """Draws a stylish car with shadows and windshield"""
    w, h = (24, 40) if vertical else (40, 24)
    pygame.draw.rect(WIN, (10, 10, 10), (x+3, y+3, w, h), border_radius=6)
    
    pygame.draw.rect(WIN, color, (x, y, w, h), border_radius=6)
    if is_emergency:
        pygame.draw.rect(WIN, WHITE, (x, y, w, h), 2, border_radius=6)
    win_c = (180, 220, 255)
    if vertical:
        pygame.draw.rect(WIN, win_c, (x+4, y+8, 16, 8), border_radius=2)
    else:
        pygame.draw.rect(WIN, win_c, (x+24, y+4, 8, 16), border_radius=2)

def draw_dashboard(data, best_road):
    pygame.draw.rect(WIN, DASHBOARD_BG, (850, 0, 250, HEIGHT))
    pygame.draw.line(WIN, GOLD, (850, 0), (850, HEIGHT), 2)

    WIN.blit(FONT_M.render("SYSTEM STATUS", True, GOLD), (870, 30))
    
    hour = data.get("hour", 0)
    time_str = f"{hour:02d}:00"
    WIN.blit(FONT_L.render(time_str, True, WHITE), (870, 60))
    
    weather_txt = "RAINY" if RAIN else "CLEAR"
    weather_col = BLUE if RAIN else GREEN
    WIN.blit(FONT_S.render(f"Weather: {weather_txt}", True, weather_col), (870, 100))

    is_rush = 17 <= hour <= 19
    density_label = "RUSH HOUR" if is_rush else "NORMAL"
    pygame.draw.rect(WIN, (50, 0, 0) if is_rush else (0, 50, 0), (870, 140, 210, 40), border_radius=5)
    WIN.blit(FONT_M.render(density_label, True, RED if is_rush else GREEN), (880, 150))

    WIN.blit(FONT_S.render(f"Vehicles Passed: {data.get('passed', 0)}", True, WHITE), (870, 220))
    
    WIN.blit(FONT_M.render("RECOMMENDATION", True, GOLD), (870, 300))
    pygame.draw.rect(WIN, (30, 30, 40), (870, 330, 210, 60), border_radius=5)
    WIN.blit(FONT_S.render(f"Open Road {best_road}", True, GREEN), (880, 340))
    WIN.blit(FONT_S.render("To Minimize Delay", True, WHITE), (880, 360))

    WIN.blit(FONT_S.render("[SPACE] Pause/Play", True, WHITE), (870, HEIGHT - 60))
    WIN.blit(FONT_S.render("[R] Toggle Rain", True, WHITE), (870, HEIGHT - 40))

def draw_scene(data):
    WIN.fill(GRASS)
    if RAIN: 
        overlay = pygame.Surface((WIDTH, HEIGHT))
        overlay.set_alpha(40)
        overlay.fill((0, 0, 50))
        WIN.blit(overlay, (0,0))

    ROAD_W = 160
    pygame.draw.rect(WIN, ASPHALT, (425 - ROAD_W//2, 0, ROAD_W, HEIGHT)) # Vert
    pygame.draw.rect(WIN, ASPHALT, (0, 400 - ROAD_W//2, 850, ROAD_W))   # Horiz
    
    pygame.draw.line(WIN, (255, 255, 100), (425, 0), (425, HEIGHT), 2)
    pygame.draw.line(WIN, (255, 255, 100), (0, 400), (850, 400), 2)

    roads = data.get("roads", [])
    green_idx = data.get("green", -1) - 1

    
    offsets = [
        (425 - 55, 400 - 120, 0, -50), 
        (425 + 100, 400 - 55, 50, 0),  
        (425 + 30, 400 + 100, 0, 50),  
        (425 - 120, 400 + 30, -50, 0)  
    ]

    max_size = -1
    best_road = 1

    for i, road in enumerate(roads):
        size = road['size']
        if size > max_size:
            max_size = size
            best_road = i + 1

        v_list = road.get("vehicles", [])
        light_color = GREEN if i == green_idx else RED
        
        sx, sy, _, _ = offsets[i]
        pygame.draw.circle(WIN, (light_color[0]//4, light_color[1]//4, light_color[2]//4), (sx+12, sy+12), 18)
        pygame.draw.circle(WIN, light_color, (sx+12, sy+12), 12)

        for idx, p in enumerate(v_list):
            ox, oy, dx, dy = offsets[i]
            vx = ox - (idx * dx)
            vy = oy - (idx * dy)
            color = YELLOW if p == 1 else BLUE
            draw_car(vx, vy, color, vertical=(i % 2 == 0), is_emergency=(p==1))

    draw_dashboard(data, best_road)

def main():
    global PAUSED, RAIN
    clock = pygame.time.Clock()
    current_data = {"roads": [], "green": -1, "hour": 9, "passed": 0}
    
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT: running = False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE: PAUSED = not PAUSED
                if event.key == pygame.K_r: RAIN = not RAIN

        if not PAUSED:
            while not output_queue.empty():
                current_data = output_queue.get()

        draw_scene(current_data)
        if PAUSED:
            s = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
            s.fill((0,0,0,150))
            WIN.blit(s, (0,0))
            WIN.blit(FONT_L.render("SYSTEM PAUSED", True, WHITE), (WIDTH//2 - 250, HEIGHT//2))

        pygame.display.flip()
        clock.tick(30)

    pygame.quit()

if __name__ == "__main__":
    main()