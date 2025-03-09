import curses
import serial
import time
import subprocess

ARDUINO_SKETCH_PATH = "/home/bee/Arduino/receive_motors"
BOARD = "arduino:avr:mega"
PORT = "/dev/ttyACM0"
BAUD_RATE = 1000000

def upload_arduino():
    try:
        print("Compiling Arduino sketch...")
        subprocess.run(f"arduino-cli compile --fqbn {BOARD} {ARDUINO_SKETCH_PATH}", shell=True, check=True)
        print("Uploading to Arduino...")
        subprocess.run(f"arduino-cli upload -p {PORT} --fqbn {BOARD} {ARDUINO_SKETCH_PATH}", shell=True, check=True)
        print("Upload successful!")
        time.sleep(2)
    except subprocess.CalledProcessError as e:
        print(f"Upload failed: {e}")
        exit(1)

def control_motors(stdscr):
    curses.curs_set(0)
    stdscr.nodelay(1)
    current_key = None
    prev_s = False
    prev_d = False
    
    try:
        arduino = serial.Serial(PORT, BAUD_RATE)
        time.sleep(2)
    except serial.SerialException as e:
        print(f"Serial connection failed: {e}")
        exit(1)

    try:
        while True:
            key = stdscr.getch()
            new_cmd = None

            # Speed controls (S/D keys)
            s_pressed = key in [ord('s'), ord('S')]
            d_pressed = key in [ord('d'), ord('D')]

            if s_pressed and not prev_s:
                arduino.write(b'F')  # Speed up
            if d_pressed and not prev_d:
                arduino.write(b'W')  # Slow down

            prev_s = s_pressed
            prev_d = d_pressed

            # Movement controls (arrow keys)
            if key == curses.KEY_UP:
                new_cmd = b'U'
            elif key == curses.KEY_DOWN:
                new_cmd = b'D'
            elif key == curses.KEY_RIGHT:
                new_cmd = b'R'
            elif key == curses.KEY_LEFT:
                new_cmd = b'L'
            elif key == 27:  # ESC to exit
                break

            # Update movement command
            if new_cmd != current_key:
                arduino.write(new_cmd if new_cmd else b'S')
                current_key = new_cmd

            time.sleep(0.01)
    except KeyboardInterrupt:
        print("\nExiting gracefully...")
    finally:
        arduino.write(b'S')  # Stop motors
        arduino.close()

if __name__ == "__main__":
    upload_arduino()
    curses.wrapper(control_motors)
