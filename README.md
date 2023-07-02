
# Setup

This requires the following libraries:

```
https://github.com/Harbys/pico-ssd1306
https://github.com/jostlowe/Pico-DMX
```

On lop and cottontail it's in `~/code` and symlinked into the project.

# Pin Mappings

| Pin | GPIO | Function | Our Use                             |
| :-: | :--: | :------- |:------------------------------------|
| 1   | GP0  | UART0 TX | Picoprobe Debugger TX               |
| 2   | GP1  | UART0 RX | Picoprobe Debugger RX               |
| 3   | GND  |          |                                     |
| 4   | GP2  | I2C1 SDA | Display SDA                         |
| 5   | GP3  | I2C1 SCL | Display SCL                         |
| 6   | GP4  | GPIO Out | Stepper Sleep Signal                |
| 7   | GP5  | GPIO In  | Stepper Fault Signal                |
| 8   | GND  |          |                                     |
| 9   | GP6  | PWM 3A   | Servo 0                             |
| 10  | GP7  | PWM 3B   | Servo 1                             |
| 11  | GP8  | PWM 4A   | Servo 2                             |
| 12  | GP9  | PWM 4B   | Servo 3                             |
| 13  | GND  |          |                                     |
| 14  | GP10 | PWM 5A   | Servo 4                             |
| 15  | GP11 | PWM 5B   | Servo 5                             |
| 16  | GP12 | PWM 6A   | Servo 6                             |
| 17  | GP13 | PWM 6B   | Servo 7                             |
| 18  | GND  |          |                                     |
| 19  | GP14 | GPIO In  | Stepper Endstop Low  (0th position) |
| 20  | GP15 | GPIO In  | Stepper Endstop High (`maxSteps`)   |
| 21  | GP16 | GPIO Out | Stepper MS1                         |
| 22  | GP17 | GPIO Out | CDC Active                          |
| 23  | GND  |          |                                     |
| 24  | GP18 | GPIO Out | Stepper Address Line 0              |
| 25  | GP19 | GPIO Out | Stepper Address Line 1              |
| 26  | GP20 | GPIO Out | Stepper Address Line 2              |
| 27  | GP21 | GPIO Out | Stepper Mux Latch                   |
| 28  | GND  |          |                                     |
| 29  | GP22 | GPIO In  | DMX In                              |
| 30  |      | Run      | Connect to ground to reset          |
| 31  | GP26 | GPIO Out | Stepper STEP Signal                 |
| 32  | GP27 | GPIO Out | Stepper DIR Signal                  |
| 33  | (A)GND | --     | (Also the analog ground)            |
| 34  | GP28 | GPIO Out | E-Stop Relay                        |
| 35  |      | ADC_VREF | Unused                              |
| 36  |      | 3V3_OUT  | ⚡️Power to the display              |
| 37  |      | 3V3_EN   |                                     |
| 38  | GND  |          | Ground Trace                        |
| 40  |      | VSYS     | 5V in, use a Schottky Diode         |
| 41  |      | VBUS     | 🚌                                  |

> **Note**: Servo pin mappings are defined in `controller-config.h`


# Power

From the docs:

> Whilst it is possible to connect the Raspberry Pi’s 5V pin to the Raspberry Pi Pico VBUS pin, this is not 
> recommended. Shorting the 5V rails together will mean that the Micro USB cannot be used. An exception is 
> when using the Raspberry Pi Pico in USB host mode, in this case 5V must be connected to the VBUS pin.
> The 3.3V pin is an OUTPUT pin on the Raspberry Pi Pico, you cannot power the Raspberry Pi Pico via this
> pin, and it should NOT be connected to a power source.

(ie, use VSYS 😅)
