import board
import pwmio
import time

servo = pwmio.PWMOut(board.GP16, duty_cycle=0, frequency=50)

MIN_DUTY = 1638
MAX_DUTY = 7864

def set_angle(angle):
    # Clamp angle between 0 and 170 as requested
    if angle < 0: angle = 0
    if angle > 170: angle = 170
    duty = int(MIN_DUTY + (angle / 170) * (MAX_DUTY - MIN_DUTY))
    servo.duty_cycle = duty

while True:
    for i in range(10, 171, 2): 
        set_angle(i)
        time.sleep(0.02)
        
    for i in range(170, 9, -2):
        set_angle(i)
        time.sleep(0.02)