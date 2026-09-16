# Troubleshooting

English | [日本語](troubleshooting.ja.md) | [Back to README](../README.en.md)

## The USB keyboard is not detected

- Install the UF2 matching your board.
- Use a data-capable USB cable, not a charge-only cable.
- Look for “Dreamcast HKT-4000 Keyboard” in the operating system.
- Confirm that the board reboots after the UF2 is copied to `RPI-RP2`.

## USB is detected, but no keys work

This means the USB side is running and the fault is on the Maple side.

1. With the HKT-4000 connected, measure approximately 5 V between 5V and GND.
2. GP10 and GP11 should each idle at approximately 3.3 V.
3. Confirm red (SDCKA) goes to GP10 and white (SDCKB) goes to GP11.
4. Connect both green (Sense) and black (GND) to GND.
5. Check the 33 ohm signal resistors, solder joints, and connector orientation.

If the connector orientation is uncertain, swap only red and white as a test;
do not move the power wires. Both are data lines, so this swap cannot put 5 V on
a GPIO.

## Single keys do nothing and combinations become modifiers

This indicates the Maple word byte-order bug in an older UF2. Reinstall the
latest `dist/dc_keyboard_usb_rp2040_zero.uf2`.

## Printed labels do not match typed characters

Select the operating-system layout matching the keyboard. Use Japanese 106/109
for a Japanese HKT-4000 and a US layout for the English model.

## A keyboard-test website does not show Right Shift

Some test sites display left/right Shift incorrectly. Turn Caps Lock off and
verify that Right Shift+A produces an uppercase `A` in a text editor.

## The RP2040-Zero RGB LED stays off

This is expected. The onboard WS2812 is not used by the current firmware.

## Input is intermittent

- Keep the signal wires short.
- Route GP10/GP11 away from ground and 5 V wiring where practical.
- Use 10–33 ohm series resistors; avoid values of 100 ohms or more.
- Check breadboard, connector, and solder-joint contact resistance.
- Add low-capacitance 3.3 V ESD protection if required.

## Checking waveforms

After enumeration, the firmware sends a Maple condition request about every
8 ms. Use a logic analyzer or oscilloscope to confirm activity on both GP10 and
GP11 and a response from the keyboard. Do not attach test equipment that is not
compatible with 3.3 V signals.
