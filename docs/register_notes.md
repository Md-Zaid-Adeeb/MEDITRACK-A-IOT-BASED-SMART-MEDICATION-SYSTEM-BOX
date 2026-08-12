# Register Notes

Quick reference for the registers used in this project (from STM32F407 reference manual).

## RCC_AHB1ENR (Clock Enable Register)
- Address: 0x40023800 + 0x30 = 0x40023830
- Bit 3 = GPIOD clock enable
- Must be set to 1 before GPIOD can be used at all — peripherals are
  disabled by default to save power

## GPIOD_MODER (Mode Register)
- Address: 0x40020C00 + 0x00 = 0x40020C00
- 2 bits per pin:
  - 00 = Input
  - 01 = General purpose output
  - 10 = Alternate function
  - 11 = Analog
- For pin N, bits are at position (N*2) and (N*2 + 1)
- PD12 uses bits 24-25 -> set to 01 for output mode

## GPIOD_ODR (Output Data Register)
- Address: 0x40020C00 + 0x14 = 0x40020C14
- Each bit corresponds to a pin's output state (0 = LOW, 1 = HIGH)
- Setting bit 12 HIGH turns ON the LED connected to PD12

## Why clock enable is needed
STM32 peripherals are disabled by default to save power. Every peripheral
(GPIO, timers, UART etc.) needs its clock enabled through the RCC registers
before its registers can be configured or used. If you skip this step,
writing to GPIOD registers has no effect — the peripheral simply isn't
powered on yet.
