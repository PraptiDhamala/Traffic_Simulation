import pygame
import subprocess
import json
import threading
import queue

pygame.init()

WIDTH, HEIGHT = 900, 600
WIN = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Traffic Simulation")

FONT = pygame.font.SysFont("Arial", 18)

ROAD_WIDTH = 120
ROAD_SPACING = 50
VEHICLE_SIZE = (20, 20)
FPS = 30

WHITE, BLACK, GRAY = (255, 255, 255), (0, 0, 0), (180, 180, 180)
BLUE, RED, GREEN, YELLOW = (50, 150, 255), (255, 50, 50), (50, 255, 50), (255, 215, 0)

output_queue = queue.Queue()

def run_c_program():
    proc = subprocess.Popen("./traffic", stdout=subprocess.PIPE, text=True)
    while True:
        line = proc.stdout.readline()
        if not line: break
        try:
            raw_data = json.loads(line.strip())
            traffic_data = {
                "roads": raw_data["roads"],
                "green_road": raw_data["green"] - 1,
                "total_passed": raw_data["passed"]
            }
            output_queue.put(traffic_data)
        except (json.JSONDecodeError, KeyError):
            continue

threading.Thread(target=run_c_program, daemon=True).start()

def draw_window(data):
    WIN.fill(WHITE)
    
    roads = data.get("roads", [])
    green_road = data.get("green_road", -1)
    total_passed = data.get("total_passed", 0)

    for i in range(4):
        road = roads[i] if i < len(roads) else {"size": 0, "vehicles": [], "avg_wait": 0}
        x = ROAD_SPACING + i * (ROAD_WIDTH + ROAD_SPACING)
        y = 50
        
        road_color = RED if road['size'] >= 50 else (GREEN if i == green_road else GRAY)
        pygame.draw.rect(WIN, road_color, (x, y, ROAD_WIDTH, HEIGHT - 150))

        priorities = road.get("vehicles", [])
        for v_idx, p in enumerate(priorities):
            vy = (HEIGHT - 100) - (v_idx * (VEHICLE_SIZE[1] + 2))
            v_color = YELLOW if p == 1 else BLUE
            pygame.draw.rect(WIN, v_color, (x + ROAD_WIDTH//2 - 10, vy - 100, 20, 20))

        txt = FONT.render(f"Road {i+1} | Q: {road['size']}", True, BLACK)
        WIN.blit(txt, (x, HEIGHT - 80))
        wait_txt = FONT.render(f"Avg Wait: {road.get('avg_wait', 0):.1f}", True, BLACK)
        WIN.blit(wait_txt, (x, HEIGHT - 60))

    passed_txt = FONT.render(f"Total Passed Vehicles: {total_passed}", True, BLACK)
    WIN.blit(passed_txt, (WIDTH - 250, 20))
    pygame.display.update()

def main():
    clock = pygame.time.Clock()
    running = True
    current_data = {} 

    while running:
        clock.tick(FPS)
        for event in pygame.event.get():
            if event.type == pygame.QUIT: running = False

        while not output_queue.empty():
            current_data = output_queue.get()

        draw_window(current_data)
    pygame.quit()

if __name__ == "__main__":
    main()