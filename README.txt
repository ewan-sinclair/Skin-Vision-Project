Notes:

## *IMPORTANT* Checklist and caveats ##

# Read this entire file before touching anything
# Always connect the ground on the controller board to ground on the arduino (pin 14).
# If you are going to power the board with no arduino connected, connect all the 5-pin arduino connectors to the GND rail. They should not float. Connect them to arduino pins or GND only.
# Do not leave the power supply connected for longer than ~5 minutes, preferably as short a time as possible. Things get too hot and start shaking the power around in scary ways.
# The +5V regulator (middle-right of the board, sticking up with a metal tab on it) gets hot very quickly, and really needs a heatsink. Watch out, it will burn you. Check its temp by touching the plastic, not the metal.
# The board likes giving electrostatic shocks sometimes. Not sure why this is. Watch out. This normally happens when gingerly checking how hot the 5V regulator has become.
# Watch the power supply for overloads. If you see the overload light on V1 or V2 (probably V2), kill the power.
# Monitor V2 on the power supply. It should be at 7V. If too many coils are on at the same time, it can end up at 13V. If this goes on for too long, kill it to be safe.

## Orientation ##

# The 'top' of the controller is the long side of the board with 3x 10-pin connectors closest to the edge. The 'bottom' is the long side with 2x 5-pin connectors closest to the edge.
# Rail voltages (Top->Bottom): | +14V*, GND* | +5V*, GND | +7V*, +14V | +5V, GND |  (* denotes the entry point for the voltage, other rails at the same voltage are bridged to the entry rails)
# The rails on the board run the same voltage all the way along the board (they are all bridged across the break in the centre)

## Colour Coding ##

Green:      Output (+/- ~7v) for the power pins
Light Blue: 5V Input (grounded vs floating/positive to switch) for the power pins
Dark blue:  +14V
Brown:      GND
Orange:     +5V
Beige:      Assorted interconnects. Also used for the +7V voltage on the power pins.

## Pins to connect on the Arduino ##

# The connectors are the 5-pin ones on the board connected to the light-blue wires coming out of the chips.
# Connectors going into the arduino should be oriented the same way as the ones one the board. Just read the colours on the board left->right, and check that they are in the same order on the arduino. This is what you'll get if you just fold the cable over without twisting it.
# If you are going to power the board with no arduino connected, connect all the 5-pin arduino connectors to the GND rail. They should not float. Connect them to arduino pins or GND only.
# Connectors are referred to like words on a page, that is left->right, down a line, left->right.
# Connectors are to be placed into these pins: Top row: | 3,4,5,6,7, 8,9,10,11,12, 52,50,48,46,44 | Bottom row: | 40,38,36,34,32, 53,51,49,47,45 |
  Note they are in groups of five, representing the connectors. Colours should connect to numbers in the left->right order you see on the board-side connection.