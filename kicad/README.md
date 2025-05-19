# PCBs

There are two different PCB types: `mainboard` and `touch-pads`.

### mainboard

- Primary board everything connects to
- Only 1 copy of the board is required

### touch-pads

- Has the touch pads and LEDs
- 4 copies of the board is required

### development

- Use KiCad 9+
- Install the [espressif libraries](https://github.com/espressif/kicad-libraries)
- Install interactive html bom plugin

### manufacturing

- The `gerber.zip` files that are committed should be enough for manufacturing
- I use JlcPcb as they seem to offer the best prices for boards larger than 100mm x 100mm
- Suggested settings:
  - FR-4
  - 2 layers
  - At least 4 boards
  - 1.6mm thickness
  - Black color
  - HASL finish
  - 1 oz cu
  - Tented vias
  - 0.3mm min via hole size
  - Remove mark on PCB
