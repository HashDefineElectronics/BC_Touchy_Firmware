#!/usr/bin/env python
# Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
# http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt

"""
usage: efm8load.py [-h] [--version] [-b BAUD] [-p PORT] [-t] bootfile

EFM8 bootloader download utility.

positional arguments:
  bootfile              boot record file to download

optional arguments:
  -h, --help            show this help message and exit
  --version             show program's version number and exit
  -b BAUD, --baud BAUD  baudrate (default: 115200)
  -p PORT, --port PORT  port (default=usb | jlink)
  -t, --trace           show download trace
"""

from __future__ import print_function
import argparse
import serial
import sys
import hidport
import smbport

VERSION = '1.10'

class Efm8HidPort(hidport.HidPort):
    """Class demonstrates how to communicate with EFM8 HID bootloader.
    
    This class provides functions for writing a boot frame and for reading 
    the response from the EFM8 HID bootloader. Because the EFM8 HID device 
    only defines one input and one output report, there is no report number.
    For Windows hosts only, read/write data exchanged with the HID driver 
    is prepended with a dummy report number.
    """
    SIZE_IN = 4     # Input (device to host) report size
    SIZE_OUT = 64   # Output (host to device) report size

    def __init__(self):
        super(Efm8HidPort, self).__init__()
        # For Windows hosts, prepend output reports with a dummy report number
        if sys.platform == 'win32':
            self.make_report = lambda r: b'\x00' + r
        else:
            self.make_report = lambda r: r

    def read(self):
        """Read the reply byte from the EFM8 bootloader.
        """
        # Remove the dummy report number if there is one
        in_report = self.get_input_report()[-Efm8HidPort.SIZE_IN:]
        # First byte of the input report has the bootloader reply
        return in_report[0:1]

    def write(self, frame):
        """Send a boot frame to the EFM8 bootolader.
        """
        # Write data in output report size chunks
        for n in range(0, len(frame), Efm8HidPort.SIZE_OUT):
            out_report = self.make_report(frame[n:n+Efm8HidPort.SIZE_OUT])
            self.set_output_report(out_report)

class Efm8SmbPort(smbport.SmbPort):
    """Class demonstrates how to communicate with EFM8 SMB bootloader.
    
    This class provides functions for writing a boot frame and for reading 
    the response from the EFM8 SMB bootloader using a CP2112-EVK.

    Args:
        address: Byte-aligned slave address.
        bitrate: SMBus clock rate in Hz.
    """
    def __init__(self, bitrate=None, address=0xF0):
        super(Efm8SmbPort, self).__init__(bitrate)
        self.address = address
        # Select the maximum write transfer size supported by the SMB master
        self.max_size = 48

    def read(self):
        """Read the reply byte from the EFM8 bootloader.
        
        The EFM8-SMB bootloader will NAK its slave address until it is finished 
        processing the previous command. The CP2112 automatically re-attempts 
        the transfer (ACK polls) until the slave ACKs. 
        """
        return self.smb_read(self.address, 1)[0:1]

    def write(self, frame):
        """Send a boot frame to the EFM8 bootloader.
        
        The CP2112 maximum write transfer is 61 bytes. So the boot frame is written 
        as several chunks. The boot frame can also be written in a single transfer 
        if the SMB master supports longer writes.
        """
        for n in range(0, len(frame), self.max_size):
            self.smb_write(self.address, frame[n:n+self.max_size])

def send_frame(port, frame):
    """Send a boot frame and return the reply.

    Args:
        frame (bytes): A boot frame to send to the EFM8 bootloader.

    Returns:
        Reply byte as an integer. 0x3F for recognized replies and timeouts.
    """
    port.write(frame)
    reply = port.read()
    if reply and reply in b'@ABC':
        return ord(reply)
    else:
        return ord(b'?')

def tohex(data):
    """Convert binary data array to ascii hex string.
    """
    return ' '.join("{:02X}".format(c) for c in data)

def efm8load(port, bootfile, trace=False):
    """Download EFM8 bootfile over the specified port.

    Args:
        port: Communication port connected to EFM8 bootloader.
        bootfile: Binary boot record file object.
        trace: If true, print trace of download data.

    Returns:
        The number of errors detected during the download.
    """
    errors = 0
    
    # Send the auto-baud training charater for the UART bootloader
    if isinstance(port, serial.Serial):
        port.write(b'\xff\xff')

    # For each frame in the binary boot record file
    header = bytearray(bootfile.read(2))
    while header and len(header) == 2:
        # Parse one boot record frame
        frame = header + bootfile.read(header[1])
        if header[0] != ord(b'$') or header[1] != len(frame)-2:
            raise RuntimeError("Bad record header")
        header = bytearray(bootfile.read(2))

        # Send the frame and record any errors
        reply = send_frame(port, frame)
        if reply != ord(b'@'):
            errors += 1

        # Display a trace record or progress bar
        if trace:
            print("{:c} {} -> {:c}".format(frame[0], tohex(frame[1:9]), reply))
        else:
            print(chr(reply), end='')
            sys.stdout.flush()

    # Return the total number of errors detected
    if not trace:
        print('')
    return errors

def open_port(port_name, baud=None):
    """Open the requested communication port.

    If the port_name is not specified, will open the first EFM8 HID or STK 
    CDC-ACM port that it finds attached.
    
    Args:
        port_name (string): Name of the communication port to open.
        baud (int): Baud rate for serial ports. Ignored for other port types.

    Returns:
        IO object for communicating with EFM8 bootloader.
    """
    if port_name is None:
        port_name = 'hid' if hidport.port_count() else 'jlink'

    if port_name.lower() in ['hid', 'usb']:
        return Efm8HidPort()
    elif port_name.lower() in ['i2c', 'smb']:
        return Efm8SmbPort(baud)
    else:
        url = "hwgrep://" + port_name
        if baud is None:
            baud = 115200
        return serial.serial_for_url(url, baud, timeout=1)

if __name__ == "__main__":
    ap = argparse.ArgumentParser(description='EFM8 bootloader download utility.')
    ap.add_argument('-v', '--version', action='version', version='%(prog)s ' + VERSION)
    ap.add_argument('bootfile', type=argparse.FileType('rb'), help='boot record file to download')
    ap.add_argument('-b', '--baud', type=int, help='baudrate (SMB:100000, UART:115200)')
    ap.add_argument('-p', '--port', help='port (default=usb | jlink)')
    ap.add_argument('-t', '--trace', action='store_true', help='show download trace')
    args = ap.parse_args()

    try:
        port = open_port(args.port, args.baud)
    except:
        sys.exit("ERROR: Unable to open port!")

    print("Download over port: {}\n".format(port.name))
    try:
        errors = efm8load(port, args.bootfile, args.trace)
        print("\nDownload complete with [ {} ] errors".format(errors))
    except KeyboardInterrupt:
        print("\nERROR: Download interrupted!")
    except RuntimeError:
        print("\nERROR: Unable to parse bootfile!")

    port.close()
