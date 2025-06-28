import socket
import pygame
import json
import os
import time

# Config:
ESP_IP = "192.168.4.1"
ESP_PORT = 4210
CONFIG_FILE = "keymap.json"

ACTIONS = [
    "L_STICK_UP", "L_STICK_DOWN", "L_STICK_LEFT", "L_STICK_RIGHT",
    "R_STICK_UP", "R_STICK_DOWN", "R_STICK_LEFT", "R_STICK_RIGHT",
    "BTN_A", "BTN_B", "BTN_X", "BTN_Y",
    "BTN_L", "BTN_R", "BTN_ZL", "BTN_ZR"
]

# Bitmask
BTN_MASKS = {
    "BTN_A": 1, "BTN_B": 2, "BTN_X": 4, "BTN_Y": 8,
    "BTN_L": 16, "BTN_R": 32, "BTN_ZL": 64, "BTN_ZR": 128
}

pygame.init()
screen = pygame.display.set_mode((600, 400))
pygame.display.set_caption("Copilot Controller")
font = pygame.font.Font(None, 36)

def draw_text(text, y_pos, color=(255, 255, 255)):
    text_surf = font.render(text, True, color)
    rect = text_surf.get_rect(center=(300, y_pos))
    screen.blit(text_surf, rect)

def run_config_wizard():
    """Interactive loop to bind keys"""
    mapping = {}
    screen.fill((50, 50, 50))
    
    for action in ACTIONS:
        waiting = True
        while waiting:
            screen.fill((50, 50, 50))
            draw_text(f"Press key for: {action}", 200, (0, 255, 255))
            pygame.display.flip()
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    exit()
                if event.type == pygame.KEYDOWN:
                    mapping[action] = event.key
                    waiting = False
                    time.sleep(0.2)
    
    # Save to file
    with open(CONFIG_FILE, "w") as f:
        json.dump(mapping, f)
    print("Configuration saved!")
    return mapping

# Main setup:
key_map = {}

# Create Config
if os.path.exists(CONFIG_FILE):
    try:
        with open(CONFIG_FILE, "r") as f:
            key_map = json.load(f)
        print("Loaded keymap.")
    except:
        print("Error loading config, running wizard...")
        key_map = run_config_wizard()
else:
    print("No config found. Running wizard...")
    key_map = run_config_wizard()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Sending to {ESP_IP}:{ESP_PORT}")
print("Press ESC to Quit. Press F1 to Reconfigure.")

running = True
while running:
    # 1. Window Events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                running = False
            if event.key == pygame.K_F1:
                key_map = run_config_wizard()

    # 2. Read Keyboard State
    keys = pygame.key.get_pressed()

    # 3. Calculate Joysticks
    lx, ly = 128, 128
    if keys[key_map["L_STICK_LEFT"]]:  lx = 0
    if keys[key_map["L_STICK_RIGHT"]]: lx = 255
    if keys[key_map["L_STICK_UP"]]:    ly = 0
    if keys[key_map["L_STICK_DOWN"]]:  ly = 255

    rx, ry = 128, 128
    if keys[key_map["R_STICK_LEFT"]]:  rx = 0
    if keys[key_map["R_STICK_RIGHT"]]: rx = 255
    if keys[key_map["R_STICK_UP"]]:    ry = 0
    if keys[key_map["R_STICK_DOWN"]]:  ry = 255

    # 4. Calculate Bitmask
    btn_byte = 0
    for action, mask in BTN_MASKS.items():
        if keys[key_map[action]]:
            btn_byte |= mask

    # 5. Send Packet (5 Bytes)
    packet = bytes([lx, ly, rx, ry, btn_byte])
    sock.sendto(packet, (ESP_IP, ESP_PORT))

    # 6. Draw Status
    screen.fill((20, 20, 20))
    draw_text(f"L Stick: {lx}, {ly}", 100)
    draw_text(f"R Stick: {rx}, {ry}", 150)
    draw_text(f"Buttons: {bin(btn_byte)}", 200)
    draw_text("Press F1 to Rebind Keys", 350, (100, 100, 100))
    pygame.display.flip()
    
    time.sleep(0.01) # 100Hz

pygame.quit()