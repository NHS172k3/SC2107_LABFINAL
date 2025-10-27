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
// Negative logic bump sensors
// P8.7 Bump5
// P8.6 Bump4
// P8.5 Bump3
// P8.4 Bump2
// P8.3 Bump1
// P8.0 Bump0

// reflectance LED illuminate connected to P5.3
// reflectance sensor 1 connected to P7.0 (robot's right, robot off road to left)
// reflectance sensor 2 connected to P7.1
// reflectance sensor 3 connected to P7.2
// reflectance sensor 4 connected to P7.3 center
// reflectance sensor 5 connected to P7.4 center
// reflectance sensor 6 connected to P7.5
// reflectance sensor 7 connected to P7.6
// reflectance sensor 8 connected to P7.7 (robot's left, robot off road to right)

#include "msp.h"
#include "..\inc\bump.h"
#include "..\inc\Reflectance.h"
#include "..\inc\Clock.h"
#include "..\inc\SysTickInts.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\TExaS.h"

volatile uint8_t reflectance_data, bump_data;
volatile uint32_t MainCount = 0;
volatile uint8_t meow = 1;
volatile uint8_t bumpRef = 0;

void SysTick_Handler(void)
{ // this is run every 1ms when the SysTick interrupt occurs
    volatile static uint8_t count = 0;
    if (count == 0)
    {
        Reflectance_Start();
    }
    else if (count == 1) // every 10ms this happens
    {
        // Polling for reflectance and bump data
        reflectance_data = Reflectance_End();
        bump_data = Bump_Read();
    }
    count++;
    if (count == 10)
        count = 0;
}







volatile uint32_t bump_val;
volatile uint32_t bump_val_hex;
volatile uint32_t index = 0;
volatile uint32_t dec_value = 0;
volatile char hex_str[2] = "FF";
volatile uint32_t hex_value = 0;

void convertManual(uint8_t bumpVal)
{
    dec_value = 0;
    hex_str[2] = "FF";
    hex_value = 0;

    bump_val = ~bumpVal; // or BumpInt_Read() if using interrupts
    bump_val_hex = ~bumpVal;


    for (int i=0; i<=5;i++){
      dec_value += (bump_val & 0x01) * (1 << i);// 1<<i is 2**i
      bump_val = bump_val >> 1;
    }
    char hex_digits[] = "0123456789ABCDEF";


    for (int i=0; i<=3;i++){
      index += (bump_val_hex & 0x01) * (1 << i);// 1<<i is 2**i
      bump_val_hex = bump_val_hex >> 1;
    }
    hex_str[1] = hex_digits[index];
    index = 0;
    for (int i=4; i<=5;i++){
      index += (bump_val_hex & 0x01) * (1 << i);// 1<<i is 2**i
      bump_val_hex = bump_val_hex >> 1;
    }
    hex_str[0] = hex_digits[index];


}

int main(void)
{

    volatile uint8_t data_pins;

    Clock_Init48MHz();
    LaunchPad_Init();
    Bump_Init(&convertManual);
    Reflectance_Init();
    TExaS_Init(LOGICANALYZER_P7);
    SysTick_Init(48000, 1); // set up SysTick for 1000 Hz interrupts
    EnableInterrupts();

    while (1)
    {
        WaitForInterrupt();        // low power mode while waiting for interrupt
        if (MainCount % 1000 == 0) //  Toggle LED every 1000 iterations (every 1000ms = 1 second)
            P2->OUT ^= 0x01;       // Heartbeat Blink
        MainCount++;

    }
}
