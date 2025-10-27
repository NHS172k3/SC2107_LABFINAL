// Lab3_Timersmain.c

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
       ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// ============================================================================
// HARDWARE CONNECTIONS
// ============================================================================
// Negative logic bump sensors (pressed = 0, not pressed = 1)
// P8.7 Bump5 (leftmost)
// P8.6 Bump4
// P8.5 Bump3
// P8.4 Bump2
// P8.3 Bump1
// P8.0 Bump0 (rightmost)

// Sever VCCMD=VREG jumper on Motor Driver and Power Distribution Board and connect VCCMD to 3.3V.
//   This makes P3.7 and P3.6 low power disables for motor drivers.  0 to sleep/stop.
// Sever nSLPL=nSLPR jumper.
//   This separates P3.7 and P3.6 allowing for independent control
// Left motor direction connected to P1.7 (J2.14)
// Left motor PWM connected to P2.7/TA0CCP4 (J4.40)
// Left motor enable connected to P3.7 (J4.31)
// Right motor direction connected to P1.6 (J2.15)
// Right motor PWM connected to P2.6/TA0CCP3 (J4.39)
// Right motor enable connected to P3.6 (J2.11)

#include "msp.h"
#include "..\inc\bump.h"
#include "..\inc\Clock.h"
#include "..\inc\SysTick.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Motor.h"
#include "..\inc\TimerA1.h"
#include "..\inc\TExaS.h"
#include "..\inc\Reflectance.h"
#include "..\inc\PWM.h"
#include "..\inc\tachometer.h"

// Global variables to store sensor data
volatile uint8_t reflectance_data;   // Reflectance sensor data (if used)
volatile uint8_t bump_data;          // Bump sensor data (0x3F = no collision)
volatile uint8_t collision_flag = 0; // Flag: 0 = no collision, 1 = collision detected

// Driver test
void TimedPause(uint32_t time)
{
  Clock_Delay1ms(time); // run for a while and stop
  Motor_Stop();
  while (LaunchPad_Input() == 0)
    ; // wait for touch
  while (LaunchPad_Input())
    ; // wait for release
}

// Test of Periodic interrupt
#define REDLED (*((volatile uint8_t *)(0x42098060)))  // P2.0 red LED
#define BLUELED (*((volatile uint8_t *)(0x42098068))) // P2.2 blue LED
uint32_t Time;

// ============================================================================
// Task() - TIMER INTERRUPT SERVICE ROUTINE
// ============================================================================
// Called by TimerA1 every 50000 cycles (about 1ms at 48MHz)
// Purpose: Check bump sensors and set collision flag
// Note: Keep this function SHORT and FAST (no delays!)
void Task(void)
{
#if 0
  // Debug code: Toggle LED to verify interrupt timing
  REDLED ^= 0x01;       // toggle P2.0
  REDLED ^= 0x01;       // toggle P2.0
  Time = Time + 1;
  REDLED ^= 0x01;       // toggle P2.0
#endif

#if 1
  volatile static uint8_t count = 0; // Counter for periodic tasks

  count++;
  if (count == 10)
    count = 0; // Reset counter every 10 interrupts (10ms)

  // Read bump sensors (0x3F = all clear, anything else = collision)
  bump_data = Bump_Read(); // Store latest bump data

  // Check for collision
  if (bump_data != 0x3F)
  {
    // COLLISION DETECTED!
    collision_flag = 1; // Set flag for main() to handle
    P3->OUT &= ~0xC0;   // Immediately put motors to sleep (emergency stop)
                        // 0xC0 = bits 6,7 (P3.6 = right motor, P3.7 = left motor)
  }
  else
  {
    // No collision
    P3->OUT |= 0xC0; // Wake up motors (allow them to run)
  }

#endif
}

// ============================================================================
// main() - MAIN PROGRAM
// ============================================================================
int main(void)
{
  // Uses Timer generated PWM to move the robot
  // Uses TimerA1 to periodically check the bump switches,
  // stopping the robot on a collision

  // Initialization
  Clock_Init48MHz();            // Set system clock to 48 MHz
  LaunchPad_Init();             // Built-in switches and LEDs
  Bump_Init();                  // Bump switches on Port 4
  Motor_Init();                 // Motor driver initialization
  TExaS_Init(LOGICANALYZER_P2); // Optional: logic analyzer for debugging
  TimerA1_Init(&Task, 50000);   // Call Task() every 50000 cycles (~1ms, 1kHz)
  EnableInterrupts();           // Enable global interrupts

  const uint32_t normalSpeed = 1000;  // PWM duty cycle for normal speed
  const uint32_t reverseSpeed = 1500; // PWM duty cycle for reversing

  // Main loop
  while (1)
  {
    // Check if collision was detected by Task() interrupt
    if (collision_flag)
    {
      // COLLISION HANDLING (runs in main loop, can use delays)

      // Step 1: Stop
      Motor_Stop();
      Clock_Delay1ms(100); // Brief pause

      // Step 2: Reverse away from obstacle
      Motor_Backward(reverseSpeed, reverseSpeed);
      Clock_Delay1ms(500); // Back up for 500ms
      Motor_Stop();
      Clock_Delay1ms(100);

      // Step 3: Turn based on which bump was hit
      if (!((bump_data & 0x01) && (bump_data & 0x02)))
      {
        // Right side collision (Bump0 or Bump1)
        Motor_Left(reverseSpeed, reverseSpeed); // Turn left
        Clock_Delay1ms(750);
      }
      else if (!((bump_data & 0x10) && (bump_data & 0x20)))
      {
        // Left side collision (Bump4 or Bump5)
        Motor_Right(reverseSpeed, reverseSpeed); // Turn right
        Clock_Delay1ms(750);
      }
      else if (!((bump_data & 0x04) && (bump_data & 0x08)))
      {
        // Center collision (Bump2 or Bump3)
        Motor_Left(reverseSpeed, reverseSpeed); // Turn left
        Clock_Delay1ms(750);
      }
      else
      {
        // Unknown pattern, turn left by default
        Motor_Left(reverseSpeed, reverseSpeed);
        Clock_Delay1ms(500);
      }

      Motor_Stop();
      Clock_Delay1ms(100);

      // Step 4: Clear collision flag and resume
      collision_flag = 0; // Clear flag (ready for next collision)
      P3->OUT |= 0xC0;    // Re-enable motors
    }
    else
    {
      // NO COLLISION - Normal forward operation
      Motor_Forward(normalSpeed, normalSpeed);
    }

    Clock_Delay1ms(10); // Small delay to prevent tight loop

    /* Original test code (commented out)
    //TimedPause(1000);
    Motor_Forward(1000,1000);  // your function
    //TimedPause(1000);

    Clock_Delay1ms(1000);
    Motor_Backward(3000,3000); // your function
    //TimedPause(1000);
    Clock_Delay1ms(1000);
    Motor_Left(3000,3000);     // your function
    //TimedPause(1000);
    Clock_Delay1ms(1000);
    Motor_Right(3000,3000);    // your function
    //TimedPause(1000);
    Clock_Delay1ms(1000);
    */
  }
}

// ============================================================================
// HOW IT WORKS:
// ============================================================================
// 1. Task() runs every ~1ms via TimerA1 interrupt
// 2. Task() reads bump sensors and sets collision_flag if collision detected
// 3. Task() immediately disables motors (emergency stop, no delay)
// 4. main() checks collision_flag in its loop
// 5. main() handles collision recovery (reverse, turn) with delays
// 6. main() clears collision_flag when done
// 7. Process repeats
//
// ADVANTAGES OF THIS APPROACH:
// - ISR stays fast (<1ms execution time)
// - System remains responsive
// - Complex collision handling in main() doesn't block interrupts
// - Easy to modify collision behavior
//
// BUMP SENSOR PATTERNS:
// 0x3F = 0b00111111 = No collision (all sensors clear)
// 0x3E = 0b00111110 = Bump0 pressed (far right)
// 0x3D = 0b00111101 = Bump1 pressed
// 0x3B = 0b00111011 = Bump2 pressed
// 0x37 = 0b00110111 = Bump3 pressed
// 0x2F = 0b00101111 = Bump4 pressed
// 0x1F = 0b00011111 = Bump5 pressed (far left)
// ============================================================================
