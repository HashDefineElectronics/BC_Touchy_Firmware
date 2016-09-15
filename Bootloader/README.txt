AN945: EFM8 Factory Bootloader Support Package
Copyright (c) 2015 by Silicon Laboratories Inc. 
=================================================

This support package is a companion to Silicon Labs App Note 495, 
"EFM8 Factory Bootloader User Guide". It provides all of the utilities,
firmware and source code referenced in the documentation.

All EFM8 devices are factory programmed with either a UART, USB, or 
SMBus bootloader that can be used for production programming or for field 
updates. AN945 provides a detailed description of the features,
design and operation of the EFM8 bootloader. It also provides a set 
of utilities for working with the bootloader and provides information 
on how to customize the bootloader if additional features are required.

The EFM8 bootloader support package is organized as follows:

AN945SW (top level)
|
|-- Bootloaders (Pre-built bootloaders for all EFM8 devices)
|-- Firmware (Firmware projects for Rainbow Blinky and the Bootloader)
\-- Tools (Utilities for generating and downloading image files)

