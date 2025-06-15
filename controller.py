import socket
import pygame
import time

# Config:
ESP_IP = "192.168.4.1" # Default IP of ESP32
ESP_PORT = 4210

print("Connecting to Copilot Controller...")
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Initialize Pygame
pygame.init()
screen = pygame.display.set_mode((400, 300))
pygame.display.set_caption("Mario Copilot - Runner Interface")

print("READY! Use WASD to Move. SHIFT to Run (B button).")

running = True
while running:
    # 1. Check Events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # 2. Read Keyboard
    keys = pygame.key.get_pressed()
    
    # Calculate X Axis (0 = Left, 128 = Center, 255 = Right)
    x_val = 128
    if keys[pygame.K_a]: x_val = 0
    elif keys[pygame.K_d]: x_val = 255
    
    # Calculate Y Axis (0 = Up, 128 = Center, 255 = Down)
    y_val = 128
    if keys[pygame.K_w]: y_val = 0
    elif keys[pygame.K_s]: y_val = 255
    
    # B Button
    b_val = 1 if (keys[pygame.K_LSHIFT] or keys[pygame.K_RSHIFT]) else 0

    # 3. Send Data to ESP32
    data = bytes([x_val, y_val, b_val])
    sock.sendto(data, (ESP_IP, ESP_PORT))

    screen.fill((30, 30, 30))
    font = pygame.font.Font(None, 36)
    text = font.render(f"X: {x_val}  Y: {y_val}  Run: {b_val}", True, (255, 255, 255))
    screen.blit(text, (50, 130))
    pygame.display.flip()

    # Rate Limit (60 FPS)
    time.sleep(0.016)

pygame.quit()