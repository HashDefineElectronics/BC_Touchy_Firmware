# BC_Touchy_Firmware
This is the project files for the Touchy board firmware. Touchy Board is the boldport club project #7.

# Author
- Ronald Sousa [@Opticalworm](https://twitter.com/Opticalworm) - [@HashDefineElec](https://twitter.com/HashDefineElec) - [www.hashdefineelectronics.com](www.hashdefineelectronics.com)

# Project Page
The project page can be found at [http://www.hashdefineelectronics.com/boldport-project-7-touchy/](http://www.hashdefineelectronics.com/boldport-project-7-touchy/)

# Setup
the project files were develop using [Silabs Simplicity studio](http://www.silabs.com/products/mcu/Pages/simplicity-studio.aspx) which is a variant of eclipse that uses Arm keil C51 compiler.

### Directories
- **HEX**  
This is where the compiled version of the projects are stored. These are purely for convenience so that anyone wanting to avoid installing and compiling the project source.

- **Simple**  
As the name suggest, this is a simple project to for test the basic functionality of the Touchy board. It uses Silabs CapsenseCircleSlider example project as its template. [Touchy](http://www.boldport.club/shop/product/437209749) board.  
**Demo Video**  
[![#5 - Demo of the test firmware for Boldport Kit no. 7 Touchy](http://img.youtube.com/vi/UFBcgWf9tuo/0.jpg)](https://youtu.be/UFBcgWf9tuo)

### IDE
This project uses [Silabs Simplicity studio](http://www.silabs.com/products/mcu/Pages/simplicity-studio.aspx).

### Tools for Debugging (Optional)
The great news is that the Touch board has its debugger ports routed out to the 2.54mm pins headers. However, the board does have a bootloader already programmed so if you have no intention to spend money on a debugger you have the choice of not using a debugger.

That said, if you are planning on doing some heavy development on the touchy board or even on the EFM8SBS1 cores then here are a couple of debugger that you can use.

- (expensive) [J-Link](http://uk.farnell.com/segger/8-08-00-j-link-base/jtag-emulator-usb-for-arm/dp/2098541?MER=bn_search_1TP_SearchNB_2) Or (cheapest) Silabs [DEBUGADPTR1-USB](http://uk.farnell.com/silicon-labs/debugadptr1-usb/adaptor-usb-debug-for-c8051fxxx/dp/1186943) 

### Bootloader
For instruction on how to program the board via the bootloader can be found [here](http://www.hashdefineelectronics.com/boldport-project-7-touchy/).

### Importing projects to Simplicity Studio
- Right Click on the Project Explorer
- Click on **Import** and then **Import** again
- Then select **General** -> **Existing Project into Workspace**
- Click **Next**
- Select the **Select root directory** and then click **browse..**
- On the Browse window, find and click on the **Simple** project directory and click **OK**
- Then click on **Finish**
