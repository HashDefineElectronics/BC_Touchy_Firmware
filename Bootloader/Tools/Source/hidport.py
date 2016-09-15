#!/usr/bin/env python
# Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
# http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt

"""USB HID communication module.

This module uses the ctypes package to wrap a small set of functions from the SiLabs HID 
Device Interface library (SLABHIDDevice.dll). The wrapped functions are all that are needed 
to communicate with the pre-loaded EFM8 USB bootloader.
"""

import ctypes as ct
import sys

# EFM8 bootloader VID/PID values
EFM8_LOADERS = [
    (0x10C4, 0xEAC9), 
    (0x10C4, 0xEACA)
]

# Dictionary maps library return codes to description string
HID_STATUS_DESC = {
    0x00 : 'HID_DEVICE_SUCCESS',
    0x01 : 'HID_DEVICE_NOT_FOUND',
    0x02 : 'HID_DEVICE_NOT_OPENED',
    0x03 : 'HID_DEVICE_ALREADY_OPENED',
    0x04 : 'HID_DEVICE_TRANSFER_TIMEOUT',
    0x05 : 'HID_DEVICE_TRANSFER_FAILED',
    0x06 : 'HID_DEVICE_CANNOT_GET_HID_INFO',
    0x07 : 'HID_DEVICE_HANDLE_ERROR',
    0x08 : 'HID_DEVICE_INVALID_BUFFER_SIZE',
    0x09 : 'HID_DEVICE_SYSTEM_CODE',
    0x0A : 'HID_DEVICE_UNSUPPORTED_FUNCTION',
    0xFF : 'HID_DEVICE_UNKNOWN_ERROR',
}

class HidError(Exception):
    """Exception class for all HID library errors.
    """
    def __init__(self, status):
        self.status = status

    def __str__(self):
        return HID_STATUS_DESC.get(self.status, 'HID_STATUS_UNKNOWN: ' + hex(self.status))

def _hid_errcheck(result, func, args):
    """ctypes errcheck function tests return code for errors.
    """
    if result != 0:
        raise HidError(result)

# Load HID shared library using ctypes
if sys.platform == 'win32':
    _DLL = ct.windll.LoadLibrary('SLABHIDDevice.dll')
elif sys.platform == 'darwin':
    _DLL = ct.cdll.LoadLibrary('libSLABHIDDevice.dylib')
else:
    raise RuntimeError('HidPort: Unsupported OS')

# Set return types and error check function for the wrapped library
_DLL.HidDevice_Open.restype = ct.c_ubyte
_DLL.HidDevice_Open.errcheck = _hid_errcheck
_DLL.HidDevice_Close.restype = ct.c_ubyte
_DLL.HidDevice_Close.errcheck = _hid_errcheck
_DLL.HidDevice_SetOutputReport_Interrupt.restype = ct.c_ubyte
_DLL.HidDevice_SetOutputReport_Interrupt.errcheck = _hid_errcheck
_DLL.HidDevice_GetInputReport_Interrupt.restype = ct.c_ubyte
_DLL.HidDevice_GetInputReport_Interrupt.errcheck = _hid_errcheck

def port_count(ids=EFM8_LOADERS):
    """Return the number of attached HID devices that match the requested ids.

    Args:
        ids: List of VID/PID tuples to match.
    """
    n = 0
    for vid, pid in ids:
        n += _DLL.HidDevice_GetNumHidDevices(vid, pid)
    return n

class HidPort(object):
    """Base class for communicating with a USB HID device.

    Creating an instance of this class automatically opens the first available 
    HID device with a matching id.

    Args:
        ids: List of VID/PID tuples to match.

    Raises:
        HidError: If a matching HID device is not available.
    """
    def __init__(self, ids=EFM8_LOADERS):
        self.handle = ct.c_void_p(0)
        self.name = ''
        for vid, pid in ids:
            if _DLL.HidDevice_GetNumHidDevices(vid, pid):
                _DLL.HidDevice_Open(ct.byref(self.handle), 0, vid, pid, 16)
                break

        if self.handle.value:
            self.name = 'HID:{:04X}:{:04X}'.format(vid, pid)
            self.len_in = _DLL.HidDevice_GetInputReportBufferLength(self.handle) & 0xFFFF
            self.len_out = _DLL.HidDevice_GetOutputReportBufferLength(self.handle) & 0xFFFF
        else:
            raise HidError(0x01)

    def close(self):
        """Close the device handle.
        """
        _DLL.HidDevice_Close(self.handle)

    def get_input_report(self, n_reports=1):
        """Receive input report(s) from the device over the interrupt endpoint.
        """
        buf = ct.create_string_buffer(n_reports * self.len_in)
        cnt = ct.c_ulong(0)
        try:
            _DLL.HidDevice_GetInputReport_Interrupt(self.handle, buf, len(buf), n_reports, ct.byref(cnt))
        except HidError as e:
            # Ignore timeout, return the data that was read
            if e.status != 0x04:
                raise
        return buf.raw[:cnt.value]

    def set_output_report(self, buffer):
        """Send an output report to the device over the interrupt endpoint.
        """
        report = ct.create_string_buffer(bytes(buffer), self.len_out)
        _DLL.HidDevice_SetOutputReport_Interrupt(self.handle, report, self.len_out)

if __name__ == "__main__":
    print('port_count() = %s' % port_count())
    try:
        port = HidPort()
        print(port.name)
        port.close()
    except HidError as e:
        print(e)
