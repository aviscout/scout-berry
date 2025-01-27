from gpiozero.pins.mock import MockFactory
from gpiozero import Button, Device
from signal import pause

class GPIOHandler:

    def __init__(self, input_pins):
        # Device.pin_factory = MockFactory()

        self.buttons = []
        self.input_pins = input_pins

        for label, pin in input_pins.items():
            try:
                btn = Button(pin, pull_up=False)
                # lambda: signal_received("hello", 42)
                btn.when_pressed = lambda pin=pin, label=label: self.signal_lost(pin, label)
                btn.when_released = lambda pin=pin, label=label: self.signal_received(pin, label)

                self.buttons.append(btn)
            except Exception as e:
                print(f"Error initializing {label}: {e}")

        # self.buttons = {label: Button(pin, pull_up=False) for label, pin in input_pins.items()}
        self.current_bearing = None
        self.current_distance = None
        self.previous_distance = None
        self.proximity = None

        self.SEGMENTS = ['SEG_A_PIN_7', 'SEG_B_PIN_6', 'SEG_C_PIN', 'SEG_D_PIN', 'SEG_E_PIN', 'SEG_F_PIN', 'SEG_G_PIN', 'SEG_DP_PIN']
        self.digit_map = {
            (1, 1, 1, 1, 1, 1, 0): 0,  # Representation of 0
            (0, 1, 1, 0, 0, 0, 0): 1,  # Representation of 1
            (1, 1, 0, 1, 1, 0, 1): 2,  # Representation of 2
            (1, 1, 1, 1, 0, 0, 1): 3,  # Representation of 3
            (0, 1, 1, 0, 0, 1, 1): 4,  # Representation of 4
            (1, 0, 1, 1, 0, 1, 1): 5,  # Representation of 5
            (1, 0, 1, 1, 1, 1, 1): 6,  # Representation of 6
            (1, 1, 1, 0, 0, 0, 0): 7,  # Representation of 7
            (1, 1, 1, 1, 1, 1, 1): 8,  # Representation of 8
            (1, 1, 1, 1, 0, 1, 1): 9   # Representation of 9
        }

        # try:
        #     self.attach_callbacks()
        # except Exception as e:
        #     print(str(e))

    def signal_lost(self, port_no, label):
        print(f"Signal LOST @ {port_no}")

    def signal_received(self, port_no, label):
        print(f"Signal RECEIVED from {label} @ {port_no}")


    def attach_callbacks(self):
        for label, button in self.buttons.items():
            button.when_pressed = lambda label=label: self.handle_input_change(label)
            button.when_released = lambda label=label: self.handle_input_change(label)

    def read_segment_values(self):
        """Read the current state of all segment pins."""
        return {segment: self.buttons[segment].is_pressed for segment in self.SEGMENTS}

    def decode_digit(self, segment_states):
        """Decode a single digit from the segment states."""
        state_tuple = tuple(segment_states[segment] for segment in self.SEGMENTS[:-1])  # Exclude DP
        return self.digit_map.get(state_tuple, -1)  # Return -1 if no match

    def decode_distance(self):
        """Decode the distance from the 7-segment inputs. Distance is in meters."""
        self.buttons['DIGIT_1_PIN'].on()
        segment_states_digit1 = self.read_segment_values()
        digit_1 = self.decode_digit(segment_states_digit1)
        self.buttons['DIGIT_1_PIN'].off()

        self.buttons['DIGIT_2_PIN'].on()
        segment_states_digit2 = self.read_segment_values()
        digit_2 = self.decode_digit(segment_states_digit2)
        self.buttons['DIGIT_2_PIN'].off()

        return digit_1 + digit_2 * 0.1

    def handle_input_change(self, label):
        """Callback function to handle changes on GPIO inputs."""
        if label.startswith('B'):  # Update bearing if it's a bearing pin
            self.current_bearing = int(label[1:])
        elif label.startswith('SEG'):  # Update distance if it's a 7-segment pin
            self.previous_distance = self.current_distance
            self.current_distance = self.decode_distance()

            if self.previous_distance is not None:
                self.proximity = self.current_distance - self.previous_distance
                if self.proximity > 0:
                    self.current_distance = -abs(self.current_distance)  # Make distance negative
                else:
                    self.current_distance = abs(self.current_distance)  # Make distance positive

        print(f"Updated State: Bearing: {self.current_bearing}, Distance: {self.current_distance}, Proximity: {self.proximity}")


class SignalHandler:
    def __init__(self, param1, param2):
        self.param1 = param1
        self.param2 = param2
        self.button = Button(17)
        self.button.when_pressed = self.signal_received

    def signal_received(self):
        print(f"Signal received with params: {self.param1}, {self.param2}")

if __name__ == "__main__":
    INPUT_PINS = {
        'B270': 17,  # Updated pin numbers (BCM)
        'B325': 27,
        'B0': 22,
        'B45': 5,
        'B90': 6,
        'SEG_A_PIN_7': 13,  # Segment A
        'SEG_B_PIN_6': 19,  # Segment B
        'SEG_C_PIN': 26,   # Segment C
        'SEG_D_PIN': 21,   # Segment D
        'SEG_E_PIN': 20,   # Segment E
        'SEG_F_PIN': 16,   # Segment F
        'SEG_G_PIN': 12,   # Segment G
        'SEG_DP_PIN': 25,  # Decimal Point
        'DIGIT_1_PIN': 8,  # Digit 1 Enable
        'DIGIT_2_PIN': 7   # Digit 2 Enable
    }

    handler = GPIOHandler(INPUT_PINS)
    print("Monitoring GPIO inputs. Press Ctrl+C to exit.")

    try:
        pause()  # Keep the script running
    except KeyboardInterrupt:
        print("\nExiting program.")

