----------------------------------------------
--  Nexys4 DDR GPIO/UART Demonstration Project  --
----------------------------------------------

******************
***  OVERVIEW  ***
******************

  The GPIO/UART Demo project demonstrates a simple usage of the Nexys4 DDR's 
  GPIO and UART. The behavior is as follows:

        *The 16 User LEDs are tied to the 16 User Switches. While the center
         User button is pressed, the LEDs are instead tied to GND
        *The 7-Segment display counts from 0 to 9 on each of its 8
         digits. This count is reset when the center button is pressed.
         Also, single anodes of the 7-Segment display are blanked by
         holding BTNU, BTNL, BTND, or BTNR. Holding the center button 
         blanks all the 7-Segment anodes.
        *An introduction message is sent across the UART when the device
         is finished being configured, and after the center User button
         is pressed.
        *A message is sent over UART whenever BTNU, BTNL, BTND, or BTNR is
         pressed.
        *The Tri-Color LEDs cycle through several colors in a ~4 second loop
        *Data from the microphone is collected and transmitted over the mono
         audio out port.
        *Note that the center user button behaves as a user reset button
         and is referred to as such in the code comments below
        
  All UART communication can be captured by attaching the UART port to a
  computer running a Terminal program with 9600 Baud Rate, 8 data bits, no 
  parity, and 1 stop bit.														