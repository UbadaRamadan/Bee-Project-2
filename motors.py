import serial
import time
import subprocess
import curses


class MotorController:
    def __init__(self):
        # Set sketch path
        self.ARDUINO_SKETCH_PATH = "/home/bee/Arduino/receive_motors"
        self.BOARD = "arduino:avr:mega"
        self.PORT = "/dev/ttyACM0"
        self.BAUD_RATE = 1000000
        self.arduino = None
        self.active_movements = {
            'U': False,
            'D': False,
            'R': False,
            'L': False,
            'H': False  # Added home toggle state
        }

    def upload_sketch(self):
        try:
            print("Compiling Arduino sketch...")
            compile_cmd = f"arduino-cli compile --fqbn {self.BOARD} {self.ARDUINO_SKETCH_PATH}"
            subprocess.run(compile_cmd, shell=True, check=True)

            print("Uploading to Arduino...")
            upload_cmd = f"arduino-cli upload -p {self.PORT} --fqbn {self.BOARD} {self.ARDUINO_SKETCH_PATH}"
            subprocess.run(upload_cmd, shell=True, check=True)
            print("Upload successful!")
            time.sleep(2)
            return True
        except subprocess.CalledProcessError as e:
            print(f"Upload failed: {e}")
            return False

    def curses_control(self):
        def _control(stdscr):
            curses.curs_set(0)  # cursor invisible
            stdscr.nodelay(1)
            try:
                while True:
                    key = stdscr.getch()
                    
                    # Arrow key toggles
                    if key == curses.KEY_UP:
                        self.active_movements['U'] = not self.active_movements['U']
                        self.send_command('U' if self.active_movements['U'] else 'S')
                        # Deactivate other X-axis movements
                        if self.active_movements['U']:
                            self.active_movements['D'] = False
                            self.active_movements['H'] = False
                    elif key == curses.KEY_DOWN:
                        self.active_movements['D'] = not self.active_movements['D']
                        self.send_command('D' if self.active_movements['D'] else 'S')
                        # Deactivate other X-axis movements
                        if self.active_movements['D']:
                            self.active_movements['U'] = False
                            self.active_movements['H'] = False
                    elif key == curses.KEY_RIGHT:
                        self.active_movements['R'] = not self.active_movements['R']
                        self.send_command('R' if self.active_movements['R'] else 'S')
                        # Deactivate other Y-axis movements
                        if self.active_movements['R']:
                            self.active_movements['L'] = False
                            self.active_movements['H'] = False
                    elif key == curses.KEY_LEFT:
                        self.active_movements['L'] = not self.active_movements['L']
                        self.send_command('L' if self.active_movements['L'] else 'S')
                        # Deactivate other Y-axis movements
                        if self.active_movements['L']:
                            self.active_movements['R'] = False
                            self.active_movements['H'] = False
                    elif key in (ord('h'), ord('H')):
                        # Toggle home movement like other arrows
                        self.active_movements['H'] = not self.active_movements['H']
                        self.send_command('H' if self.active_movements['H'] else 'S')
                        # Deactivate other movements when homing
                        if self.active_movements['H']:
                            self.active_movements['U'] = False
                            self.active_movements['D'] = False
                            self.active_movements['R'] = False
                            self.active_movements['L'] = False
                    elif key == 27:  # ESC key
                        # Stop all movements
                        self.send_command('S')
                        for cmd in self.active_movements:
                            self.active_movements[cmd] = False
                        break

                    time.sleep(0.05)  # Small delay to prevent overwhelming the serial port
                    
            except KeyboardInterrupt:
                print("\nExiting gracefully...")
                self.send_command('S')

        if self.connect():
            curses.wrapper(_control)
            self.emergency_stop()

    def connect(self):
        try:
            self.arduino = serial.Serial(self.PORT, self.BAUD_RATE)
            time.sleep(2)
            return True
        except serial.SerialException as e:
            print(f"Connection failed: {e}")
            return False

    def send_command(self, command):
        if self.arduino and self.arduino.is_open:
            try:
                self.arduino.write(command.encode())
                return True
            except serial.SerialException:
                return False
        return False

    def receive_command(self, timeout=1.0):
        if not self.arduino or not self.arduino.is_open:
            return None

        original_timeout = self.arduino.timeout
        self.arduino.timeout = timeout

        try:
            response = self.arduino.readline().decode().strip()
            return response if response else None
        except (serial.SerialException, UnicodeDecodeError):
            print("Cannot receive command")
            return None
        finally:
            self.arduino.timeout = original_timeout

    def emergency_stop(self):
        self.send_command('S')
        if self.arduino:
            self.arduino.close()


if __name__ == "__main__":
    controller = MotorController()
    if controller.upload_sketch():
        controller.curses_control()
