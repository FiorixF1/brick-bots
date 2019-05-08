# brick-bots
This repository contains a flashable firmware for the [Elegoo Smart Car Robot Kit](https://www.amazon.it/Elegoo-Ultrasuoni-Bluetooth-Intelligente-Educativo/dp/B078X7HLWN).

This project has been created during BrickBots, a course to learn to program robots with Arduino. The goal of the course was creating a robot that could detect objects on a table and make them fall (without falling itself, obviously).

## Wiring

The wiring of the components is the same found in the official manual of the robot, with a small difference. Given the goal of our robot, we did not use the Line Tracking Module. Instead we have put three infrared proximity sensors with the aim of detecting the presence of an object and the table surface.

The three sensors are put in front of the car, such that the one in the middle points towards the table (it detects if the table is present or not), while the other two are put on the sides of the car such that they can detect an object in the near proximity. The sonar sensor is used as well to detect more distant objects, and it is part of the original kit.

To make sure the infrared sensors behave as they should, they need to be linked to the following pins:
* Front sensor at pin 2
* Left sensor at pin 10
* Right sensor at pin 4

The firmware has been flashed and tested on an Arduino UNO. It should work on any Arduino board which supports interrupts on pin 2.