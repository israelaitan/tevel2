SAM-BA(R) 2.12 Multiple Programming Example
 
1.Introduction

This example shows how to use Sam-BA efficiently for the mass production of devices supported by Sam-BA.
There are three use cases:
    Case 1: Scan devices supported by Sam-BA continuously, and automatic run a predefined Sam-BA script to program the devices.
    Case 2: Automatic program all the connected devices at a single click of button using a predefined Sam-BA script.
    Case 3: Select one of connect devices and program it with a predefined Sam-BA script. 

Case 1 and Case 2 are frequently used in real production. 
Case 3 could be consider a test tool for doing customization of script files. 

In all the cases, we assume:
    All the devices are programmed by a specified script file.
    The progress of programming is shown as percentage of template log file, called: template.log
    In case of programming failed, we are able to kill the selected task.

- Runs under Windows XP, Windows Vista 32-bits, Windows Vista 64-bits, Windows 7 32-bits and 
  Windows 7 64-bits.
- Support multiple programming
  - Up to 8 target boardss can be programmed from a single PC via USB serial port. 
    - User is able to expand to much more devices.
  - Embedded command line executable with settable script file.
    - Take sam9g15 in this example
    - User is able to program all devices in sam-ba board list by modifying script file.
  - Allows users to customize algorithms (applets) for memory that we don't already support.
  - Using a template log file to show the progress 
  - Support auto and manual mode.
  - The dead thread can be killed automaticlly.
  - Simple PASS / FAIL response on display.
  
2. Script files

  - Bootfiles folder contains all script files and example binaries.
  - sam9gx5.bat & killtask.bat is able to launcth sam-ba.exe with parameter.
  - sam9gx5.tcl is script file to program on board serial flash.
  - dummy.tcl is internal script file. (Don¡¯t modify it)
  - example.bin is a example boot file which can be replaced with user bootfile.
  - template.log is the result from (sam9gx5.tcl and example.bin).
  - Other log files (COM25.log, COM31.log¡­) are generated in run time.

3. How to modify script files?
  - How to modify batch file?
    - Syntax:
        sam-ba.exe %1 BOARD_NAME bootfiles\SCRIPT_NAME > bootfiles\%2 2>&1

    - Example:
        sam-ba.exe %1 AT91SAM9N12-EK bootfiles\sam9n12.tcl > bootfiles\%2 2>&1

  - How to modify tcl script file.
    - sam9gx5.tcl is an example to program on board serial flash.
    - User is able to program all supported external memories for specific board
    - For example : to program NAND flash for sam9n12 
      - all useful scripts is already presented in ..\tcl_lib\at91sam9n12-ek\at91sam9n12-ek.tcl
      - NANDFLASH::Init
      - NANDFLASH::NandHeaderValue HEADER 0xc0c00405
      - NANDFLASH::EraseAllNandFlash
      - NANDFLASH::SendBootFilePmeccCmd $bootstrapFile
      - send_file {NandFlash} "$ubootFile" $ubootAddr 0
      - send_file {NandFlash} "$kernelFile" $kernelAddr 0
      - send_file {NandFlash} "$rootfsFile" $rootfsAddr 0

4. How to generate templat.log?
    - If the sam9n12.tcl is ready(tcl example as above).
    - Connect target board with USB port (\USBserial\COM25 for an example).
    - Execute command line to generate template.log (to replace original file)
      sam-ba.exe \USBserial\COM25 AT91SAM9N12-EK bootfiles\sam9n12.tcl > bootfiles\template.log 2>&1
    
5. Launch the example
  - Auto mode
    - Auto detect device(s) which connect with usb port and start to program the device automaticlly.
  - Manual mode
    - Click the button ¡°Scan devices¡± to scan all connected devices.
    - Click the button ¡°Program all devices¡± to program all connected devices.
    - Select one device in devices list, and double click this device to program on dedicated device. 
  - Status 
    - Progress bar shows the rate of progress for each devices.
    - If all devices have been programmed successfully, it report ¡°pass¡± for each devices.
    - If one of the devices can¡¯t be programmed properly lead to timeout, the task will be killed.
