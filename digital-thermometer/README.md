
# Digital Thermometer

Displays temperature on two 7-segment displays.

## Parts:

- 1x Arduino UNO
- 2x 7-Segment Displays
- 2x NPN-Transistors(BC547)
- 1x Temperaturesensor(TMP36)
- 8x 220 ohm-Resistors
- 2x 1k ohm-Resistors

### TMP36

Voltage = (Sensor-Value / 1024) * 5

Temperature(°C) = (Voltage - 0.5) * 100

### 7-Segment-Display Pin Layout

![7-Segment_Display_Visual_Pinout_Diagram.png](203px-7-Segment_Display_Visual_Pinout_Diagram.png)

#### My Pin-Layout

PINS | LED-Position
-----|-------------
Q0   | middle
Q1   | top left
Q2   | top
Q3   | top right
Q4   | bottom left
Q5   | bottom
Q6   | bottom right
Q7   | dot

Source:
[https://commons.wikimedia.org/wiki/File:7-Segment_Display_Visual_Pinout_Diagram.svg](https://commons.wikimedia.org/wiki/File:7-Segment_Display_Visual_Pinout_Diagram.svg)
