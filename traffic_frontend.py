import pygame
import subprocess
import json
import threading
import queue
import math

pygame.init()
WIDTH, HEIGHT = 1100, 800  
WIN = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("SmartCity Traffic Controller")


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
OVERRIDE_ROAD = -1

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
    WIN.blit(FONT_L.render(f"{hour:02d}:00", True, WHITE), (870, 60))
    
    weather_txt = "RAINY" if RAIN else "CLEAR"
    WIN.blit(FONT_S.render(f"Weather: {weather_txt}", True, BLUE if RAIN else GREEN), (870, 100))

    WIN.blit(FONT_M.render(" Recommendation ", True, GOLD), (870, 180))
    pygame.draw.rect(WIN, (30, 30, 40), (870, 210, 210, 60), border_radius=5)
    WIN.blit(FONT_S.render(f"Open Road {best_road}", True, GREEN), (880, 220))
    
    WIN.blit(FONT_M.render("LIVE ANALYTICS", True, GOLD), (870, 320))
    pygame.draw.rect(WIN, (30, 30, 40), (870, 350, 210, 80), border_radius=5)
    WIN.blit(FONT_S.render(f"Delayed ID: {data.get('worst_id', 0)}", True, WHITE), (880, 365))
    WIN.blit(FONT_S.render(f"Wait: {data.get('max_wait', 0)} ticks", True, RED), (880, 390))

    WIN.blit(FONT_S.render("[SPACE] Pause", True, WHITE), (870, HEIGHT - 80))
    WIN.blit(FONT_S.render("[R] Toggle Rain", True, WHITE), (870, HEIGHT - 60))
    WIN.blit(FONT_S.render("Click Road to Override", True, YELLOW), (870, HEIGHT - 40))

def draw_scene(data):
    WIN.fill(GRASS)
    if RAIN:
        overlay = pygame.Surface((WIDTH, HEIGHT)); overlay.set_alpha(40); overlay.fill((0,0,50)); WIN.blit(overlay, (0,0))

    ROAD_W = 160
    pygame.draw.rect(WIN, ASPHALT, (425 - ROAD_W//2, 0, ROAD_W, HEIGHT))
    pygame.draw.rect(WIN, ASPHALT, (0, 400 - ROAD_W//2, 850, ROAD_W))
    pygame.draw.line(WIN, (255, 255, 100), (425, 0), (425, HEIGHT), 2)
    pygame.draw.line(WIN, (255, 255, 100), (0, 400), (850, 400), 2)

    active_green = OVERRIDE_ROAD if OVERRIDE_ROAD != -1 else data.get("green", -1)
    roads = data.get("roads", [])
    offsets = [(425-55, 400-120, 0, -50), (425+100, 400-55, 50, 0), (425+30, 400+100, 0, 50), (425-120, 400+30, -50, 0)]
    
    max_size, best_road = -1, 1
    for i, road in enumerate(roads):
        if road['size'] > max_size: max_size = road['size']; best_road = i + 1
        light_color = GREEN if (i+1) == active_green else RED
        sx, sy, _, _ = offsets[i]
        pygame.draw.circle(WIN, light_color, (sx+12, sy+12), 12)
        for idx, p in enumerate(road.get("vehicles", [])):
            ox, oy, dx, dy = offsets[i]
            draw_car(ox - (idx * dx), oy - (idx * dy), YELLOW if p == 1 else BLUE, (i%2==0), (p==1))
    draw_dashboard(data, best_road)

def main():
    global PAUSED, RAIN, OVERRIDE_ROAD
    clock, current_data = pygame.time.Clock(), {"roads": [], "green": -1, "hour": 9, "passed": 0}
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT: return
            if event.type == pygame.MOUSEBUTTONDOWN:
                mx, my = pygame.mouse.get_pos()
                if 400 < mx < 450 and 375 < my < 425: OVERRIDE_ROAD = -1
                else:
                    if my < 350: OVERRIDE_ROAD = 1
                    elif mx > 500: OVERRIDE_ROAD = 2
                    elif my > 450: OVERRIDE_ROAD = 3
                    elif mx < 350: OVERRIDE_ROAD = 4
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE: PAUSED = not PAUSED
                if event.key == pygame.K_r: RAIN = not RAIN
        if not PAUSED and not output_queue.empty():
            while not output_queue.empty(): current_data = output_queue.get()
        draw_scene(current_data)
        pygame.display.flip()
        clock.tick(30)

if __name__ == "__main__": main()