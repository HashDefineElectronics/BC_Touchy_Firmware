#!/usr/bin/env python
# Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
# http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt

"""CP2112 HID to SMBus bridge interface.

This module uses the ctypes package to wrap a small set of functions from the Silicon Labs 
HID to SMBus Interface library (SLABHIDtoSMBus.dll). The wrapped functions are all that are 
needed to communicate with the EFM8 SMB bootloader.
"""

import ctypes as ct
import sys

# Status (S0) codes
class HID_SMBUS_S0:
    IDLE = 0x00
    BUSY = 0x01
    COMPLETE = 0x02
    ERROR = 0x03

# Detailed status (S1) codes
class HID_SMBUS_S1:
    BUSY_ADDRESS_ACKED = 0x00
    BUSY_ADDRESS_NACKED = 0x01
    BUSY_READING = 0x02
    BUSY_WRITING = 0x03
    ERROR_TIMEOUT_NACK = 0x00
    ERROR_TIMEOUT_BUS_NOT_FREE = 0x01
    ERROR_ARB_LOST = 0x02
    ERROR_READ_INCOMPLETE = 0x03
    ERROR_WRITE_INCOMPLETE = 0x04
    ERROR_SUCCESS_AFTER_RETRY = 0x05

# Dictionary maps library return codes to description string
HID_SMBUS_STATUS_DESC = {
    0x00 : "HID_SMBUS_SUCCESS",
    0x01 : "HID_SMBUS_DEVICE_NOT_FOUND",
    0x02 : "HID_SMBUS_INVALID_HANDLE",
    0x03 : "HID_SMBUS_INVALID_DEVICE_OBJECT",
    0x04 : "HID_SMBUS_INVALID_PARAMETER",
    0x05 : "HID_SMBUS_INVALID_REQUEST_LENGTH",
    0x10 : "HID_SMBUS_READ_ERROR",
    0x11 : "HID_SMBUS_WRITE_ERROR",
    0x12 : "HID_SMBUS_READ_TIMED_OUT",
    0x13 : "HID_SMBUS_WRITE_TIMED_OUT",
    0x14 : "HID_SMBUS_DEVICE_IO_FAILED",
    0x15 : "HID_SMBUS_DEVICE_ACCESS_ERROR",
    0x16 : "HID_SMBUS_DEVICE_NOT_SUPPORTED",
    0xFF : "HID_SMBUS_UNKNOWN_ERROR",
}

class HidSmbusError(Exception):
    """Exception class for all HIDtoSMBus library errors.
    """
    def __init__(self, status):
        self.status = status

    def __str__(self):
        return HID_SMBUS_STATUS_DESC.get(self.status, 'SMB_STATUS_UNKNOWN: ' + hex(self.status))

def hidsmb_errcheck(result, func, args):
    """ctypes errcheck function tests return code for errors.
    """
    if result != 0:
        raise HidSmbusError(result)

# Load HID shared library using ctypes
if sys.platform == 'win32':
    _DLL = ct.windll.LoadLibrary("SLABHIDtoSMBus.dll")
elif sys.platform == 'darwin':
    _DLL = ct.cdll.LoadLibrary("libSLABHIDtoSMBus.dylib")
else:
    raise RuntimeError("HidSmbus: Unsupported OS")

# Set return types and error check function for the wrapped library
for hidsmb_function in [
    "HidSmbus_GetNumDevices", 
    "HidSmbus_Open", 
    "HidSmbus_Close", 
    "HidSmbus_ReadRequest", 
    "HidSmbus_GetReadResponse",
    "HidSmbus_WriteRequest", 
    "HidSmbus_TransferStatusRequest", 
    "HidSmbus_GetTransferStatusResponse", 
    "HidSmbus_SetSmbusConfig"]:
    fnc = getattr(_DLL, hidsmb_function)
    fnc.restype = ct.c_int
    fnc.errcheck = hidsmb_errcheck

def port_count():
    """Return the number of attached Silicon Labs CP2112 devices.
    """
    ndev = ct.c_ulong()
    _DLL.HidSmbus_GetNumDevices(ct.byref(ndev), 0x10C4, 0xEA90)
    return ndev.value

class SmbPort(object):
    """Base class for communicating with a Silicon Labs CP2112 device.

    Creating an instance of this class automatically opens the first available 
    CP2112 device.

    Args:
        bitrate: SMBus clock rate in Hz (default: 100000).

    Raises:
        HidError: If a CP2112 device is not available.
    """
    def __init__(self, bitrate=None):
        self.handle = ct.c_void_p(0)
        self.name = 'SMB:CP2112'
        if bitrate is None:
            bitrate = 100000
        if port_count():
            _DLL.HidSmbus_Open(ct.byref(self.handle), 0, 0x10C4, 0xEA90)
            _DLL.HidSmbus_SetSmbusConfig(self.handle, bitrate, 0x02, True, 500, 500, False, 0)
        else:
            raise HidSmbusError(0x01)

    def close(self):
        """Close the device handle.
        """
        _DLL.HidSmbus_Close(self.handle)

    def smb_status(self):
        """Return SMB transfer status information.

        Returns:
            Tuple(status, detailedStatus, numRetries, bytesRead)
        """
        status_0 = ct.c_byte(0)
        status_1 = ct.c_byte(0)
        tries = ct.c_ushort(0)
        count = ct.c_ushort(0)
        _DLL.HidSmbus_TransferStatusRequest(self.handle)
        _DLL.HidSmbus_GetTransferStatusResponse(self.handle, ct.byref(status_0), ct.byref(status_1), ct.byref(tries), ct.byref(count))
        return (status_0.value, status_1.value, tries.value, count.value)

    def smb_read(self, address, count=64):
        """Read a stream of bytes from a SMB slave device.

        Args:
            address: Byte-aligned slave address.
            count: Number of bytes to read (range: 1-512).

        Returns:
            Buffer with the data that was read.
        """
        _DLL.HidSmbus_ReadRequest(self.handle, (address & 0xFE), count)
        size = max(count, 64)
        buf = ct.create_string_buffer(size)
        status = ct.c_byte(0)
        n_read = ct.c_byte(0)
        try:
            _DLL.HidSmbus_GetReadResponse(self.handle, ct.byref(status), buf, size, ct.byref(n_read))
        except HidSmbusError as e:
            # Ignore timeout, return the data that was read
            if e.status != 0x12:
                raise
        return buf.raw[:n_read.value]

    def smb_write(self, address, buffer, count=None):
        """Write a stream of bytes to a SMB slave device.

        Args:
            address: Byte-aligned slave address.
            buffer: Buffer with data to write.
            count: Number of bytes to write (range: 1-61).
        """
        if count is None:
            count = len(buffer)
        _DLL.HidSmbus_WriteRequest(self.handle, (address & 0xFE), bytes(buffer), count)
        while self.smb_status()[0] == HID_SMBUS_S0.BUSY:
            pass

if __name__ == "__main__":
    print('port_count() = %s' % port_count())
    try:
        port = SmbPort()
        print(port.name)
        port.close()
    except HidSmbusError as e:
        print(e)
