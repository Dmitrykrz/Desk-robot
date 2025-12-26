# Desk Robot Companion

![IMG_5923](https://github.com/user-attachments/assets/427c614b-8066-4b08-952f-eb4805b5ad16)



## What is it for?

As with all my projects, I build first and ask questions later. Now that it exists, the answer is clear: this little companion sits on my desk, waves at me when I return from coffee breaks, performs choreographed dances when bored, and generally serves as the world's most overengineered desk buddy.

It detects when I'm present, judges me silently with its LED face, and occasionally spins around dramatically for no reason.

Productivity tool? Emotional support robot? Procrastination enabler? Yes.


Due to the polarizing film with diffuser, the screen is notoriously difficult to capture on camera. It looks far better in person than in any photo here.
The low-ISO image below better represents the real appearance. I love this retro yellow vibe. Each pixel transforms from a point into a vertical rectangle, an artifact of two polarizing films cut at an angle. It gives the face a warmth and texture that photos simply cannot convey.

![IMG_5931](https://github.com/user-attachments/assets/2dc9bba1-147f-4863-be73-f2a9dac1b311)

## Project Structure

- **PCB** — Files and photos for the custom circuit board
- **Hardware** — Fusion 360 design files for the robot body
- **Software** — Arduino code for ESP32-C6

## How It Works

### The Spin

The robot rotates on a large bearing with a 3mm steel shaft running through its center, connecting base to body. A hollow slip ring wraps around this shaft, passing USB power from the stationary base to the spinning upper body—unlimited rotation, no tangled wires, maximum drama.

The base motor drives through a 1:34 gearbox for smooth, theatrical rotations.
<img width="800" height="624" alt="internal cut" src="https://github.com/user-attachments/assets/b57c43de-1bd4-44ad-8eb7-b1036d8c02ab" />



### The Wave

Both arms use tiny stepper motors with integrated worm gears, giving them that slow, deliberate wave that says *"I acknowledge your existence, human."*

### The Brain

Three TMC2209 stepper drivers live on a custom PCB, commanded by an ESP32-C6. The firmware runs a timeline-based animation system—choreographed sequences of motor movements and facial expressions, all scheduled down to the millisecond.

![Desk robot (2)](https://github.com/user-attachments/assets/aef4f20e-cfd0-488d-808b-e4010ad919cb)

![Desk robot (7)](https://github.com/user-attachments/assets/5db32d9b-cd12-439d-aecb-3df1c374eb85)



### The Awareness

A radar module hides behind the face, detecting humans up to 8 meters away. When you approach, it wakes. When you leave, it sulks. The radar's wide spread means no sneaking past unnoticed.

### The Face

An Adafruit Charlieplexed LED matrix—warm yellow, delightfully retro. This display started everything. I saw its glow, imagined a tiny robot companion with emotions rendered in chunky pixels, and the rest became inevitable.


