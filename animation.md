# Desk Robot Animation System

## Overview

Timeline-based animation system for ESP32 desk robot with 3 stepper motors and LED matrix face. Commands are scheduled by millisecond timestamps and execute non-blocking in the main loop.

## Hardware

- **base**: Rotating body (stepper motor)
- **left**: Left arm (stepper motor)
- **right**: Right arm (stepper motor)
- **screen**: 8x16 LED matrix displaying emotions (images loaded from Preferences storage)

## Animation JSON Format

Send via MQTT to topic `desk_robot`:
```json
{
  "animation": [
    {"ms": 0, "motor": "base", "pos": 200, "spd": 400, "acc": 800},
    {"ms": 0, "motor": "left", "pos": 100, "spd": 600, "acc": 1200},
    {"ms": 500, "motor": "left", "pos": -100},
    {"ms": 1000, "screen_name": "happy"}    
  ]
}
```

## Command Fields

### Motor commands (base, left, right)

| Field | Required | Description |
|-------|----------|-------------|
| `ms` | Yes | Time in milliseconds from animation start |
| `motor` | Yes | Motor: `"base"`, `"left"`, or `"right"` |
| `pos` | Yes | Target position in steps |
| `spd` | No | Speed (steps/sec). If omitted, retains previous value |
| `acc` | No | Acceleration (steps/sec²). If omitted, retains previous value |

### Screen commands

| Field | Required | Description |
|-------|----------|-------------|
| `ms` | Yes | Time in milliseconds from animation start |
| `screen` | Yes (or use `screen_name` or `screen_data`) | Image name saved in Preferences (legacy) |
| `screen_name` | Yes (or use `screen` or `screen_data`) | Image name saved in Preferences |
| `screen_data` | Yes (or use `screen` or `screen_name`) | Raw 128-character image data string (8×16, `0` or `1`) |

**Note**: Use either `screen`/`screen_name` (to load saved image) OR `screen_data` (to display raw data without saving).

## Behavior

- Commands must be sorted by `ms` value (ascending)
- Commands with same `ms` value execute together
- Motors operate independently and simultaneously
- Speed/acceleration persist per motor until explicitly changed
- Animation completes when all commands dispatched and all motors stopped
- Maximum 50 commands per animation

## MQTT Responses

Published to `desk_robot_send`:

- `{"animation_started": true, "commands": N}` — animation begun
- `{"animation_complete": true}` — animation finished
- `{"animation_stopped": true}` — animation halted by command

## Animation Control
```json
{"animation_stop": 1}
```
Immediately halts animation and all motors.

## Direct Motor Control

### Base motor

| Command | Description |
|---------|-------------|
| `{"base_move": 200}` | Move base to position 200 |
| `{"base_speed": 400}` | Set speed (also sets acceleration to speed × 2) |
| `{"base_acc": 800}` | Set acceleration independently |
| `{"base_zero": 1}` | Set current position as zero (no movement) |

### Left arm motor

| Command | Description |
|---------|-------------|
| `{"left_move": 200}` | Move left arm to position (-1000 to 1000) |
| `{"left_speed": 400}` | Set speed (also sets acceleration to speed × 2) |
| `{"left_acc": 800}` | Set acceleration independently |
| `{"left_zero": 1}` | Set current position as zero (no movement) |

### Right arm motor

| Command | Description |
|---------|-------------|
| `{"right_move": 200}` | Move right arm to position (-1000 to 1000) |
| `{"right_speed": 400}` | Set speed (also sets acceleration to speed × 2) |
| `{"right_acc": 800}` | Set acceleration independently |
| `{"right_zero": 1}` | Set current position as zero (no movement) |

### All motors

| Command | Description |
|---------|-------------|
| `{"all_zero": 1}` | Set all motors' current position as zero |
| `{"all_home": 1}` | Move all motors to position 0 |

## Screen/Image Commands

### Display image directly

Send 128-character string (8 rows × 16 columns, `0` or `1`): must be 128 long

example of smile face:
{"image":"00100000000001000101000000001010010100000000101001010000000010100010000000000100000000000000000000001000000100000000011111100000"}
example sad face:
{"image":"00100000000001000101000000001010010100000000101000100000000001000000000000000000000001111110000000001100001100000001000000001000"}

### Save current image to storage
```json
{"image_save": "myimage"}
```
Image name must be 1-15 characters. Maximum 10 saved images.

### Load and display saved image
```json
{"image_show": "myimage"}
```

### List all saved images
```json
{"image_show_all": true}
```
Returns: `{"saved_images": ["image1", "image2", ...]}`

### Delete saved image
```json
{"image_delete": "myimage"}
```

## Radar Control

| Command | Description |
|---------|-------------|
| `{"radarenable": 1}` | Enable radar human detection |
| `{"radarenable": 0}` | Disable radar human detection |

When enabled, publishes `{"human": 1}` or `{"human": 0}` on detection changes.

## System Commands

| Command | Description |
|---------|-------------|
| `{"command": "reset"}` | Restart the ESP32 |

## Example Animations

### Animation with raw image data (no saving required)
```json
{
  "animation": [
    {"ms": 0, "screen_data": "00100000000001000101000000001010010100000000101001010000000010100010000000000100000000000000000000001000000100000000011111100000"},
    {"ms": 500, "screen_data": "00100000000001000101000000001010010100000000101000100000000001000000000000000000000001111110000000001100001100000001000000001000"},
    {"ms": 1000, "screen_name": "smile1"}
  ]
}
```

### Simple wave
```json
{
  "animation": [
    {"ms": 0, "motor": "left", "pos": 200, "spd": 800, "acc": 1600},
    {"ms": 300, "motor": "left", "pos": -200},
    {"ms": 600, "motor": "left", "pos": 200},
    {"ms": 900, "motor": "left", "pos": -200},
    {"ms": 1200, "motor": "left", "pos": 0}
  ]
}
```

### Dance (base rotation with arm waves)
```json
{
  "animation": [
    {"ms": 0, "screen": "happy"},
    {"ms": 0, "motor": "base", "pos": 300, "spd": 100, "acc": 200},
    {"ms": 0, "motor": "left", "pos": 200, "spd": 800, "acc": 1600},
    {"ms": 0, "motor": "right", "pos": -200, "spd": 800, "acc": 1600},
    {"ms": 400, "motor": "left", "pos": -200},
    {"ms": 400, "motor": "right", "pos": 200},
    {"ms": 800, "motor": "left", "pos": 200},
    {"ms": 800, "motor": "right", "pos": -200},
    {"ms": 1200, "motor": "left", "pos": -200},
    {"ms": 1200, "motor": "right", "pos": 200},
    {"ms": 1600, "motor": "left", "pos": 0},
    {"ms": 1600, "motor": "right", "pos": 0},
    {"ms": 2000, "motor": "base", "pos": 0},
    {"ms": 2500, "screen": "smile"}
  ]
}
```

### Excited greeting (fast arm movement while nodding)
```json
{
  "animation": [
    {"ms": 0, "screen": "excited"},
    {"ms": 0, "motor": "left", "pos": 300, "spd": 1000, "acc": 2000},
    {"ms": 0, "motor": "right", "pos": 300, "spd": 1000, "acc": 2000},
    {"ms": 0, "motor": "base", "pos": 50, "spd": 200, "acc": 800},
    {"ms": 200, "motor": "left", "pos": 100},
    {"ms": 200, "motor": "right", "pos": 100},
    {"ms": 200, "motor": "base", "pos": -50},
    {"ms": 400, "motor": "left", "pos": 300},
    {"ms": 400, "motor": "right", "pos": 300},
    {"ms": 400, "motor": "base", "pos": 50},
    {"ms": 600, "motor": "left", "pos": 100},
    {"ms": 600, "motor": "right", "pos": 100},
    {"ms": 600, "motor": "base", "pos": -50},
    {"ms": 800, "motor": "left", "pos": 0},
    {"ms": 800, "motor": "right", "pos": 0},
    {"ms": 800, "motor": "base", "pos": 0},
    {"ms": 1000, "screen": "smile"}
  ]
}
```

## Jog Commands (Relative Movement)

Jog commands move motors relative to their current position, unlike `*_move` commands which use absolute positioning.

| Command | Description |
|---------|-------------|
| `{"base_jog": 100}` | Move base 100 steps from current position |
| `{"base_jog": -100}` | Move base -100 steps from current position |
| `{"left_jog": 50}` | Move left arm 50 steps from current position |
| `{"left_jog": -50}` | Move left arm -50 steps from current position |
| `{"right_jog": 50}` | Move right arm 50 steps from current position |
| `{"right_jog": -50}` | Move right arm -50 steps from current position |

Jog commands are useful for manual positioning and fine-tuning without tracking absolute positions.