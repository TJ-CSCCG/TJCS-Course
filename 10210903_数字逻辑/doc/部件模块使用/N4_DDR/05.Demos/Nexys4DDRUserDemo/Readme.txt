----------------------------------------------
--  Nexys4-DDR User Demo ISE Project            --
----------------------------------------------
Copyright 2014, Digilent Inc.

******************
***  OVERVIEW  ***
******************

  This project implements the User Demo loaded on the Nexys4-DDR at the factory, and demonstrates 
usage of the VGA display in 1280 X 1024 mode, the Artix7 XADC Temperature sensor, the ADT7420 
Temperature Sensor, the ADXL362 Accelerometer, a USB Mouse (using the PS2 interface), the RGB Leds, 
the ADMP421 Omnidirectional Microphone, the PWM Audio Output, the soft SRAM interface to DDR, 
the user buttons, the user switches, and the user LEDs.

  The project was created under ISE 14.7.

  The behavior is as follows:

  The project connects to the VGA display in a 1280*1024 resolution and displays various
  items on the screen:

    - a Digilent / Analog Devices logo

    - a mouse cursor, if a USB mouse is connected to the board when the project is started

    - the audio signal from the onboard ADMP421 Omnidirectional Microphone

    - a small square representing the X and Y acceleration data from the ADXL362 onboard Accelerometer.
      The square moves according the Nexys4-DDR board position. Note that the X and Y axes 
      on the board are exchanged due to the accelerometer layout on the Nexys4-DDR board.
      The accelerometer display also displays the acceleration magnitude, calculated as
      SQRT( X^2 + Y^2 +Z^2), where X, Y and Z represent the acceleration value on the respective axes

    - The FPGA temperature, the ADT7420 temperature and the accelerometer temperature values

    - The value of the R, G and B components sent to the RGB Leds LD16 and LD17

   Other features:

    - The 16 Switches (SW0..SW15) are connected to LD0..LD15 except when audio recording is done

    - Pressing BTNL, BTNC and BTNR will toggle between Red, Green and Blue colors on LD16 and LD17
      Color sweeping returns when BTND is pressed. BTND also toggles between LD16, LD17, none or both

    - Pressing BTNU will start audio recording for about 5S, then the audio data will be played back
      on the Audio output. While recording, LD15..LD0 will show a progress bar moving to left, while
      playing back, LD15..LD0 will show a progress bar moving to right. Recorded audio data is stored in
      the onboard DDR via an emulated SRAM interface. 

********************
*** KNOWN ISSUES ***
********************

  - The accelerometer's temperature sensor does not provide an accurate absolute temperature reading, but does accurately
    measure changes in temperature. It is possible to compensate for the offset between the reported temperature and the
    actual temperature, but this demo does not do this, because the offset is different for every Nexys4-DDR. This means the 
    temperature display for the accelerometer reports an inaccurate temperature value.

  - Certain mice will not work with this demo. This has to do with their initialization requirements. Currently the 
    only work around for this is to try to use a different mouse.
	
  - The Block RAMs that hold the ROMs that contain the overlay and logo data are instantiated inside of a blackbox
    netlist, and therefore can't be modified. The netlists were generated with a coregen project that is not included
    with this demo. If you need to rebuild the BRAMs you will need to download the Vivado version of this project, which
    is available on the Nexys4 DDR wiki page. That version instantiates the BRAMs using inference and the BRAM wizard.
	
	

