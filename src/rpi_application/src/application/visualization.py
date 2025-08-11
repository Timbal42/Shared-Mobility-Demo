import time
import RPi.GPIO as GPIO

led_front_left = 35
led_front_right = 36
led_rear_left = 37
led_rear_right = 38
doors_open = 15
doors_close = 16


def gpioInit():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setwarnings(False)
    # set up the GPIO channels - one input and one output
    
    GPIO.setup(led_front_left, GPIO.OUT)
    GPIO.setup(led_front_right, GPIO.OUT)
    GPIO.setup(led_rear_left, GPIO.OUT)
    GPIO.setup(led_rear_right, GPIO.OUT)
    
    GPIO.setup(doors_open, GPIO.OUT)
    GPIO.setup(doors_close, GPIO.OUT)
    
    GPIO.output(led_front_left, 0)
    GPIO.output(led_front_right, 0)
    GPIO.output(led_rear_right, 0)
    GPIO.output(led_rear_left, 0)
    GPIO.output(doors_open, 0)
    GPIO.output(doors_close, 0)
    
def showReady():
    print("[VIS] show ready...")
    
    GPIO.output(led_front_left, 1)
    GPIO.output(led_front_right, 1)
    GPIO.output(led_rear_right, 1)
    GPIO.output(led_rear_left, 1)
    time.sleep(0.8)
    GPIO.output(led_front_left, 0)
    GPIO.output(led_front_right, 0)
    GPIO.output(led_rear_right, 0)
    GPIO.output(led_rear_left, 0)
    time.sleep(0.8)
    GPIO.output(led_front_left, 1)
    GPIO.output(led_front_right, 1)
    GPIO.output(led_rear_right, 1)
    GPIO.output(led_rear_left, 1)
    time.sleep(0.8)
    GPIO.output(led_front_left, 0)
    GPIO.output(led_front_right, 0)
    GPIO.output(led_rear_right, 0)
    GPIO.output(led_rear_left, 0)
    time.sleep(0.8)
    GPIO.output(led_front_left, 1)
    GPIO.output(led_front_right, 1)
    GPIO.output(led_rear_right, 1)
    GPIO.output(led_rear_left, 1)
    time.sleep(0.8)
    GPIO.output(led_front_left, 0)
    GPIO.output(led_front_right, 0)
    GPIO.output(led_rear_right, 0)
    GPIO.output(led_rear_left, 0)

    

def showWorking():
    print("[VIS] show working...")
    
    GPIO.output(led_front_left, 1)
    GPIO.output(led_front_right, 1)
    time.sleep(0.2)
    GPIO.output(led_front_left, 0)
    GPIO.output(led_front_right, 0)
    time.sleep(0.2)
    GPIO.output(led_rear_right, 1)
    GPIO.output(led_rear_left, 1)
    time.sleep(0.2)
    GPIO.output(led_rear_right, 0)
    GPIO.output(led_rear_left, 0)
    time.sleep(0.2)
    
def closeDoors():
    print("[VIS] close doors...")
    GPIO.output(led_front_left, 1)
    GPIO.output(led_front_right, 1)
    GPIO.output(led_rear_right, 1)
    GPIO.output(led_rear_left, 1)
    GPIO.output(doors_close, 1)
    time.sleep(3)
    GPIO.output(doors_close, 0)
    GPIO.output(led_front_left, 0)
    GPIO.output(led_front_right, 0)
    GPIO.output(led_rear_right, 0)
    GPIO.output(led_rear_left, 0)
    print("[VIS] done")
        
def openDoors():
    print("[VIS] open doors...")
    GPIO.output(led_front_left, 1)
    GPIO.output(led_front_right, 1)
    GPIO.output(led_rear_right, 1)
    GPIO.output(led_rear_left, 1)
    GPIO.output(doors_open, 1)
    time.sleep(3)
    GPIO.output(doors_open, 0)
    GPIO.output(led_front_left, 0)
    GPIO.output(led_front_right, 0)
    GPIO.output(led_rear_right, 0)
    GPIO.output(led_rear_left, 0)
    print("[VIS] done")



