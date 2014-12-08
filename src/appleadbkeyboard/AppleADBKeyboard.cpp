/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * 18 June 1998
 * Start IOKit version.
 */

#include "AppleADBKeyboard.h"
#include <IOKit/hidsystem/IOHIDTypes.h>
#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#define super IOHIKeyboard
OSDefineMetaClassAndStructors(AppleADBKeyboard,IOHIKeyboard)


static void new_kbd_data ( IOService * us, UInt8 adbCommand, IOByteCount length, UInt8 * data );
static void asyncSetLEDFunc ( thread_call_param_t, thread_call_param_t );

#if 0  //The following table is in Info.plist now
//Convert raw ADB codes to MacOS 9 KMAP virtual key codes in dispatchKeyboardEvent()
static unsigned char	kmapConvert[] = 
	{
	//00,00,00,00,  These 4 are in System resource, but are unused
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x3B,0x37,0x38,0x39,0x3A,0x7B,0x7C,0x7D,0x7E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x3C,0x3D,0x3E,0x36,0x7F,
	00,00
	}; 
#endif


// **********************************************************************************
// start
//
// **********************************************************************************
bool AppleADBKeyboard::start ( IOService * theNub )
{
    OSString *		data;
    const char *	pTable;
    OSNumber 		*enable_fwd_delete;

    enable_fwd_delete = OSDynamicCast( OSNumber, getProperty("PowerBook fn Foward Delete"));
    if (enable_fwd_delete)
    {
	_enable_fwd_delete = (bool) enable_fwd_delete->unsigned32BitValue();
    }
   
    adbDevice = (IOADBDevice *)theNub;
    if( !adbDevice->seizeForClient(this, new_kbd_data) ) {
	    IOLog("%s: Seize failed\n", getName());
	    return false;
    }
    
    //Get virtual key map table
    data = OSDynamicCast( OSString, getProperty( "ADBVirtualKeys" ));
    if (data)
    {
	pTable = data->getCStringNoCopy();
	if (data->getLength() < (5 * 0x7f)) //7f codes, 5 bytes per code in XML string
	{
	    IOLog("AppleADBKeyboard: too few virtual keys found in Info.plist");
	    for (int i=0; i<128; i++)
	    {
		_virtualmap[i] = i;	//Somewhat usable keymap
	    }
	} 
	else //pTable could get access violation if not all bytes are accounted for
	{
	    for (int i=0; i< 128; i++) //No codes greater than 0x7f allowed in packet()
	    {
		_virtualmap[i] = strtol(pTable, NULL, 16); //Must be base 16
		pTable += 5;  //Format is "0x00,0x01,0x02"
	    }
	}
    }
    else
    {
	return false; 
    }
    
    _get_last_keydown = OSSymbol::withCString("get_last_keydown");
    _get_handler_id = OSSymbol::withCString("get_handler_id");
    _get_device_flags = OSSymbol::withCString("get_device_flags");
    
    turnLEDon = ADBKS_LED_CAPSLOCK | ADBKS_LED_NUMLOCK | ADBKS_LED_SCROLLLOCK; //negative logic
    setAlphaLockFeedback(false);
    setNumLockFeedback(false);
    
    clock_interval_to_absolutetime_interval( 4, kSecondScale, &rebootTime);
    clock_interval_to_absolutetime_interval( 1, kSecondScale, &debuggerTime);
    updateFKeyMap();  //UPDATE PMU DATA IF APPROPRIATE 
    
    return super::start(theNub);
}

// --------------------------------------------------------------------------
//
// Method: updateFKeyMap
//
// Purpose:  Update the Fkey map in PMU with the values from the name
//   	     registry, if needed.  This came from the UpdateFKeyMap routine
//	     in OS9 Excelsior:OS:Wallyworld sources.       
/* static */ void
AppleADBKeyboard::updateFKeyMap()
{
    IORegistryEntry		*devicetreeRegEntry;
    typedef char  	keystrtype[4];
    unsigned long 	buttonkeyvalue; 	// button translation value from name registry
    unsigned char	fkeyindex;		// f key number being checked
    keystrtype mapentry[] = 
					{
					{"F0"},  {"F1"},  {"F2"},  {"F3"},  {"F4"},  {"F5"},  {"F6"},
							{"F7"},  {"F8"},  {"F9"},  {"F10"},  {"F11"},  {"F12"}, 
					};
    devicetreeRegEntry = fromPath("mac-io/via-pmu/adb/keyboard", gIODTPlane);
    if(devicetreeRegEntry != NULL)  {
	if (OSDynamicCast(OSData, devicetreeRegEntry->getProperty("AAPL,has-embedded-fn-keys"))) {
            for (fkeyindex = 1; fkeyindex <= 12;  fkeyindex++)
                {
		OSData *tmpData = OSDynamicCast(OSData, devicetreeRegEntry->getProperty(mapentry[fkeyindex]));
		if(tmpData != NULL) {
			memcpy(&buttonkeyvalue, (UInt8 *)tmpData->getBytesNoCopy(), sizeof(buttonkeyvalue));
			setButtonTransTableEntry(fkeyindex, buttonkeyvalue);
			}
		}
            }
	}
}

// ******************************************************************************************************
// Method:  setButtonTransTableEntry
//
// Purpose:	Used for local write to PMU adb interface (to set up PMU registers).
//		This is essentially a writeToDevice routine (as in ADB device).
//		The command string typically looks like (on the ApplePMU driver
//		sendMiscCommand side) as an example: 
//		ADBcmd ADBcnt IntKbdCmd AutoPollbit ADBcount PMUcount selector  F12key keyvalue
//		  20      07      28        02         04      03       01        0c      8b
// ******************************************************************************************************
/* static */ void
AppleADBKeyboard::setButtonTransTableEntry(unsigned char fkeynum, unsigned char transvalue)
{						// CMD: int KBD 2 reg 0 listen filled in by IOPMUADBController
    UInt8 oBuffer[3];				// embed. data count =3 to PMU filled in by IOPMUADBController
    oBuffer[0] = 1;				// selector for PMU: set fkey command
    oBuffer[1] = fkeynum;			// fkey to set
    oBuffer[2] = transvalue;			// button code to attach
    IOByteCount oLength = sizeof(oBuffer);
    adbDevice->writeRegister(0, oBuffer, &oLength);   // adbRegister = 0, IOADBDevice adds addr of 0x02 => 0x28
}	

// **********************************************************************************
// interfaceID
//
// **********************************************************************************
UInt32 AppleADBKeyboard::interfaceID ( void )
{
return NX_EVS_DEVICE_INTERFACE_ADB;
}


// **********************************************************************************
// deviceType
//
// **********************************************************************************
UInt32 AppleADBKeyboard::deviceType ( void )
{
    UInt32	id;	//We need handler ID to remap adjustable JIS keyboard
    IORegistryEntry 	*regEntry;
    OSData 		*data = 0;
    UInt32 		*dataptr;
    OSNumber 		*xml_handlerID;

    xml_handlerID = OSDynamicCast( OSNumber, getProperty("alt_handler_id"));
    if (xml_handlerID)
    {
	id = xml_handlerID->unsigned32BitValue();
    }
    else
    {
	id = adbDevice->handlerID();
    }

    if (id == 18)  //Adjustable JIS
    {
	_virtualmap[0x32] = 0x35; //tilde to ESC
    }

    if ((id == kgestaltPwrBkEKDomKbd) || (id == kgestaltPwrBkEKISOKbd) || 
	(id == kgestaltPwrBkEKJISKbd) || (id == kgestaltPwrBk99JISKbd))
    {	
	if( (regEntry = IORegistryEntry::fromPath( "/pci@f2000000/mac-io/via-pmu/adb/keyboard", gIODTPlane ))) 
	{
	    data = OSDynamicCast(OSData, regEntry->getProperty( "keyboard-id", gIODTPlane, kIORegistryIterateRecursively ));
	    if (data)
	    {
		dataptr = (UInt32 *)data->getBytesNoCopy();
		id = *dataptr; //make sure no byte swapping
	    }
	    regEntry->release();
	}
    }    

    return id;
}


// **********************************************************************************
// setAlphaLockFeedback
// This is usually called on a call-out thread after the caps-lock key is pressed.
// ADB operations to PMU are synchronous, and this is must not be done
// on the call-out thread since that is the PMU driver workloop thread, and
// it will block itself.
//
// Therefore, we schedule the ADB write to disconnect the call-out thread
// and the one that initiates the ADB write.
//
// **********************************************************************************
void AppleADBKeyboard::setAlphaLockFeedback ( bool to )
{    
    if (to)
	turnLEDon &= ~ADBKS_LED_CAPSLOCK; //Inverse logic applies here
    else
	turnLEDon |= ADBKS_LED_CAPSLOCK;

    if ( ! isInactive() ) {
	thread_call_func(asyncSetLEDFunc, (thread_call_param_t)this, true);
    }
}

void AppleADBKeyboard::setNumLockFeedback ( bool to )
{
     if (to) //LED on means clear that bit
	turnLEDon &= ~ ADBKS_LED_NUMLOCK; 
    else
	turnLEDon |= ADBKS_LED_NUMLOCK;

    if ( ! isInactive() ) {
	thread_call_func(asyncSetLEDFunc, (thread_call_param_t)this, true);
    }

}



// **********************************************************************************
// asyncSetLEDFunc
//
// Called asynchronously to turn on/off the capslock and numlock LED
//
// **********************************************************************************
static void asyncSetLEDFunc ( thread_call_param_t self, thread_call_param_t )
{
    
UInt16		value;
IOByteCount	length = sizeof( UInt16);


    value = ((AppleADBKeyboard*)self)->turnLEDon;
    ((AppleADBKeyboard*)self)->adbDevice->writeRegister(2, (UInt8 *)&value, &length);
}

/**********************************************************************
Get LED status by reading hardware.  Register 2 has 16 bits.
**********************************************************************/
unsigned AppleADBKeyboard::getLEDStatus (void )
{  
    UInt8       data[8];  //8 bytes max for ADB read (talk) operation
    IOByteCount length = 8;

    bzero(data, 8);
    LEDStatus = 0;
    adbDevice->readRegister(2, data, &length);

    if ((data[1] & ADBKS_LED_NUMLOCK) == 0)
	LEDStatus |= ADBKS_LED_NUMLOCK;
    if ((data[1] & ADBKS_LED_CAPSLOCK) == 0)
	LEDStatus |= ADBKS_LED_CAPSLOCK;
    if ((data[1] & ADBKS_LED_SCROLLLOCK) == 0)
	LEDStatus |= ADBKS_LED_SCROLLLOCK;
  
    return LEDStatus;
}

// **********************************************************************************
// new_kbd_data
//
// **********************************************************************************
static void new_kbd_data ( IOService * us, UInt8 adbCommand, IOByteCount length, UInt8 * data )
{
((AppleADBKeyboard *)us)->packet(data,length,adbCommand);
}

// **********************************************************************************
// dispatchKeyboardEvent
//
// **********************************************************************************
extern "C" {
void Debugger( const char * );
void boot(int paniced, int howto, char * command);
#define RB_HALT		0x08	/* don't reboot, just halt */
}

static void AppleADBKeyboardReboot( thread_call_param_t arg, thread_call_param_t )
{
    boot( 0, (int) arg, 0 );
}

void AppleADBKeyboard::dispatchKeyboardEvent(unsigned int	keyCode,
                            /* direction */ bool         	goingDown,
                            /* timeStamp */ AbsoluteTime	time)
{
    if( !goingDown && programmerKey) {
	programmerKey = false;
	EVK_KEYUP( ADBK_CONTROL, _keyState);
	SUB_ABSOLUTETIME( &time, &programmerKeyTime );
        if( CMP_ABSOLUTETIME( &time, &rebootTime) >= 0) {
            
	    thread_call_func( AppleADBKeyboardReboot,
				(void *) RB_HALT, true );
	} else if( CMP_ABSOLUTETIME( &time, &debuggerTime) >= 0) {
	    Debugger("Programmer Key");
	}

    } 
    else if (keyCode == ADBK_POWER)
    {
	if (EVK_IS_KEYDOWN( ADBK_CONTROL, _keyState)) {

	    if( !programmerKey) {
		programmerKey = true;
		programmerKeyTime = time;
	    }
	    return;
	}
	else if (deviceFlags() & NX_SECONDARYFNMASK)
	{ 
	    return;  //On PowerBooks fn + CMD creates a POWER keycode
	}
    }

    if ((_enable_fwd_delete) && (deviceFlags() & NX_SECONDARYFNMASK))
    {
	if (keyCode == ADBK_DELETE)
	{
	    keyCode = ADBK_FORWARD_DELETE;
	    _fwd_delete_down = goingDown;
	}
	else if (keyCode == ADBK_PBFNKEY)
	{
	    if ((!goingDown) && (_fwd_delete_down))
	    {
		super::dispatchKeyboardEvent( _virtualmap[ADBK_FORWARD_DELETE], 
		    false, time );
		_fwd_delete_down = false;
	    }
	}
    
    }

    super::dispatchKeyboardEvent( _virtualmap[keyCode], goingDown, time );

}

// **********************************************************************************
// packet
//
// **********************************************************************************
IOReturn AppleADBKeyboard::packet (UInt8 * data, IOByteCount, UInt8 adbCommand )
{
unsigned int	keycode1, keycode2;
bool		down;
AbsoluteTime	now;

keycode1 = *data;
down = ((keycode1 & 0x80) == 0);
keycode1 &= 0x7f;
if(keycode1 == 0x7e) keycode1 = ADBK_POWER;
clock_get_uptime(&now);
setTimeLastNonmodKeydown(now, keycode1);
dispatchKeyboardEvent(keycode1,down,now);

keycode2 = *(data + 1);
if( keycode2 != 0xff ) {
        down = ((keycode2 & 0x80) == 0);
        keycode2 &= 0x7f;
        if( keycode2 == 0x7e) keycode2 = ADBK_POWER;
	if( (keycode1 != ADBK_POWER) || (keycode2 != ADBK_POWER))
		dispatchKeyboardEvent(keycode2,down,now);
}

return kIOReturnSuccess;
}

//This is needed by trackpads to correct jitter when gestures
//  are enabled.
void AppleADBKeyboard::setTimeLastNonmodKeydown (AbsoluteTime now, unsigned int keycode)
{
    switch (_virtualmap[keycode]) {
	case  0x3b:	//left control, not raw
	case  ADBK_FLOWER:	
	case  ADBK_SHIFT:	
	case  ADBK_SHIFT_R:    
	case  ADBK_CAPSLOCK:	
	case  ADBK_OPTION:	
	case  ADBK_OPTION_R:
	case  ADBK_PBFNKEY:	//PowerBook secondary fn key
	    break;
	default:
	    _lastkeydown = now;
	    break;
    }
}

AbsoluteTime AppleADBKeyboard::getTimeLastNonmodKeydown (void)
{
    if (CMP_ABSOLUTETIME( &_lastkeyCGEvent, &_lastkeydown) > 0)
     	return _lastkeyCGEvent;  //time of autorepeat
    else
	return _lastkeydown;     //time of physical keypress
}

//The only reason this method is subclassed is to access autorepeat
//  events without changing IOHIKeyboard's public APIs.  All normal
//  as well as autorepeated keys must come through here
void AppleADBKeyboard::keyboardEvent(unsigned eventType,
	/* flags */              unsigned flags,
	/* keyCode */            unsigned keyCode,
	/* charCode */           unsigned charCode,
	/* charSet */            unsigned charSet,
	/* originalCharCode */   unsigned origCharCode,
	/* originalCharSet */    unsigned origCharSet)
{

    if (_codeToRepeat != (unsigned)-1)  //only save time if autorepeated key
    {
    	//save my own time, ignore super's time.   
	clock_get_uptime(&_lastkeyCGEvent);  
    }
    super::keyboardEvent( eventType, flags, keyCode, charCode, charSet,
	origCharCode, origCharSet);
}

IOReturn AppleADBKeyboard::callPlatformFunction(const OSSymbol *functionName,
						    bool waitForFunction,
						    void *param1, void *param2,
						    void *param3, void *param4)
{  
    if (functionName == _get_last_keydown)
    {
	AbsoluteTime *timeptr;
	
	timeptr = (AbsoluteTime *)param1;
	*timeptr = getTimeLastNonmodKeydown();
	return kIOReturnSuccess;
    }
    
    if (functionName == _get_handler_id)
    {
	UInt32	*id;
	
	id = (UInt32 *)param1;
	*id = deviceType();
	return kIOReturnSuccess;
    }

    if (functionName == _get_device_flags)
    {
	UInt32	*id;
	
	id = (UInt32 *)param1;
	//TBD:  check if keyboard is external or embedded?
	*id = deviceFlags();
	return kIOReturnSuccess;
    }
    
    
    return kIOReturnBadArgument;
}


// **********************************************************************************
// maxKeyCodes
//
// **********************************************************************************
UInt32 AppleADBKeyboard::maxKeyCodes ( void )
{
return 0x80;
}

//Get key values from ev_keymap.h
bool AppleADBKeyboard:: doesKeyLock ( unsigned key)
{
    switch (key) {
	case NX_KEYTYPE_CAPS_LOCK:
		return true;
	case NX_KEYTYPE_NUM_LOCK:
		return false;
	default:
		return false;
    }
}


// **********************************************************************************
// defaultKeymapOfLength
//
// **********************************************************************************
const unsigned char * AppleADBKeyboard::defaultKeymapOfLength (UInt32 * length )
{
static const unsigned char appleUSAKeyMap[] = {
            0x00,0x00,
	    0x08,	//8 modifier keys
	    0x00,0x01,0x39,  //NX_MODIFIERKEY_ALPHALOCK
	    0x01,0x01,0x38,  //NX_MODIFIERKEY_SHIFT virtual from KMAP
	    0x02,0x01,0x3b,  //NX_MODIFIERKEY_CONTROL
	    0x03,0x01,0x3a,  //NX_MODIFIERKEY_ALTERNATE
	    0x04,0x01,0x37,	  //NX_MODIFIERKEY_COMMAND
	    0x05,0x15,0x52,0x41,0x4c,0x53,0x54,0x55,0x45,0x58,0x57,0x56,0x5b,0x5c,
            0x43,0x4b,0x51,0x7b,0x7d,0x7e,0x7c,0x4e,0x59,  //NX_MODIFIERKEY_NUMERICPAD
	    0x06,0x01,0x72, //NX_MODIFIERKEY_HELP  7th modifier here
	    0x07,0x01,0x3f, //NX_MODIFIERKEY_SECONDARYFN 8th modifier
	    0x7f,0x0d,0x00,0x61,
            0x00,0x41,0x00,0x01,0x00,0x01,0x00,0xca,0x00,0xc7,0x00,0x01,0x00,0x01,0x0d,0x00,
            0x73,0x00,0x53,0x00,0x13,0x00,0x13,0x00,0xfb,0x00,0xa7,0x00,0x13,0x00,0x13,0x0d,
            0x00,0x64,0x00,0x44,0x00,0x04,0x00,0x04,0x01,0x44,0x01,0xb6,0x00,0x04,0x00,0x04,
            0x0d,0x00,0x66,0x00,0x46,0x00,0x06,0x00,0x06,0x00,0xa6,0x01,0xac,0x00,0x06,0x00,
            0x06,0x0d,0x00,0x68,0x00,0x48,0x00,0x08,0x00,0x08,0x00,0xe3,0x00,0xeb,0x00,0x00,
            0x18,0x00,0x0d,0x00,0x67,0x00,0x47,0x00,0x07,0x00,0x07,0x00,0xf1,0x00,0xe1,0x00,
            0x07,0x00,0x07,0x0d,0x00,0x7a,0x00,0x5a,0x00,0x1a,0x00,0x1a,0x00,0xcf,0x01,0x57,
            0x00,0x1a,0x00,0x1a,0x0d,0x00,0x78,0x00,0x58,0x00,0x18,0x00,0x18,0x01,0xb4,0x01,
            0xce,0x00,0x18,0x00,0x18,0x0d,0x00,0x63,0x00,0x43,0x00,0x03,0x00,0x03,0x01,0xe3,
            0x01,0xd3,0x00,0x03,0x00,0x03,0x0d,0x00,0x76,0x00,0x56,0x00,0x16,0x00,0x16,0x01,
            0xd6,0x01,0xe0,0x00,0x16,0x00,0x16,0x02,0x00,0x3c,0x00,0x3e,0x0d,0x00,0x62,0x00,
            0x42,0x00,0x02,0x00,0x02,0x01,0xe5,0x01,0xf2,0x00,0x02,0x00,0x02,0x0d,0x00,0x71,
            0x00,0x51,0x00,0x11,0x00,0x11,0x00,0xfa,0x00,0xea,0x00,0x11,0x00,0x11,0x0d,0x00,
            0x77,0x00,0x57,0x00,0x17,0x00,0x17,0x01,0xc8,0x01,0xc7,0x00,0x17,0x00,0x17,0x0d,
            0x00,0x65,0x00,0x45,0x00,0x05,0x00,0x05,0x00,0xc2,0x00,0xc5,0x00,0x05,0x00,0x05,
            0x0d,0x00,0x72,0x00,0x52,0x00,0x12,0x00,0x12,0x01,0xe2,0x01,0xd2,0x00,0x12,0x00,
            0x12,0x0d,0x00,0x79,0x00,0x59,0x00,0x19,0x00,0x19,0x00,0xa5,0x01,0xdb,0x00,0x19,
            0x00,0x19,0x0d,0x00,0x74,0x00,0x54,0x00,0x14,0x00,0x14,0x01,0xe4,0x01,0xd4,0x00,
            0x14,0x00,0x14,0x0a,0x00,0x31,0x00,0x21,0x01,0xad,0x00,0xa1,0x0e,0x00,0x32,0x00,
            0x40,0x00,0x32,0x00,0x00,0x00,0xb2,0x00,0xb3,0x00,0x00,0x00,0x00,0x0a,0x00,0x33,
            0x00,0x23,0x00,0xa3,0x01,0xba,0x0a,0x00,0x34,0x00,0x24,0x00,0xa2,0x00,0xa8,0x0e,
            0x00,0x36,0x00,0x5e,0x00,0x36,0x00,0x1e,0x00,0xb6,0x00,0xc3,0x00,0x1e,0x00,0x1e,
            0x0a,0x00,0x35,0x00,0x25,0x01,0xa5,0x00,0xbd,0x0a,0x00,0x3d,0x00,0x2b,0x01,0xb9,
            0x01,0xb1,0x0a,0x00,0x39,0x00,0x28,0x00,0xac,0x00,0xab,0x0a,0x00,0x37,0x00,0x26,
            0x01,0xb0,0x01,0xab,0x0e,0x00,0x2d,0x00,0x5f,0x00,0x1f,0x00,0x1f,0x00,0xb1,0x00,
            0xd0,0x00,0x1f,0x00,0x1f,0x0a,0x00,0x38,0x00,0x2a,0x00,0xb7,0x00,0xb4,0x0a,0x00,
            0x30,0x00,0x29,0x00,0xad,0x00,0xbb,0x0e,0x00,0x5d,0x00,0x7d,0x00,0x1d,0x00,0x1d,
            0x00,0x27,0x00,0xba,0x00,0x1d,0x00,0x1d,0x0d,0x00,0x6f,0x00,0x4f,0x00,0x0f,0x00,
            0x0f,0x00,0xf9,0x00,0xe9,0x00,0x0f,0x00,0x0f,0x0d,0x00,0x75,0x00,0x55,0x00,0x15,
            0x00,0x15,0x00,0xc8,0x00,0xcd,0x00,0x15,0x00,0x15,0x0e,0x00,0x5b,0x00,0x7b,0x00,
            0x1b,0x00,0x1b,0x00,0x60,0x00,0xaa,0x00,0x1b,0x00,0x1b,0x0d,0x00,0x69,0x00,0x49,
            0x00,0x09,0x00,0x09,0x00,0xc1,0x00,0xf5,0x00,0x09,0x00,0x09,0x0d,0x00,0x70,0x00,
            0x50,0x00,0x10,0x00,0x10,0x01,0x70,0x01,0x50,0x00,0x10,0x00,0x10,0x10,0x00,0x0d,
            0x00,0x03,0x0d,0x00,0x6c,0x00,0x4c,0x00,0x0c,0x00,0x0c,0x00,0xf8,0x00,0xe8,0x00,
            0x0c,0x00,0x0c,0x0d,0x00,0x6a,0x00,0x4a,0x00,0x0a,0x00,0x0a,0x00,0xc6,0x00,0xae,
            0x00,0x0a,0x00,0x0a,0x0a,0x00,0x27,0x00,0x22,0x00,0xa9,0x01,0xae,0x0d,0x00,0x6b,
            0x00,0x4b,0x00,0x0b,0x00,0x0b,0x00,0xce,0x00,0xaf,0x00,0x0b,0x00,0x0b,0x0a,0x00,
            0x3b,0x00,0x3a,0x01,0xb2,0x01,0xa2,0x0e,0x00,0x5c,0x00,0x7c,0x00,0x1c,0x00,0x1c,
            0x00,0xe3,0x00,0xeb,0x00,0x1c,0x00,0x1c,0x0a,0x00,0x2c,0x00,0x3c,0x00,0xcb,0x01,
            0xa3,0x0a,0x00,0x2f,0x00,0x3f,0x01,0xb8,0x00,0xbf,0x0d,0x00,0x6e,0x00,0x4e,0x00,
            0x0e,0x00,0x0e,0x00,0xc4,0x01,0xaf,0x00,0x0e,0x00,0x0e,0x0d,0x00,0x6d,0x00,0x4d,
            0x00,0x0d,0x00,0x0d,0x01,0x6d,0x01,0xd8,0x00,0x0d,0x00,0x0d,0x0a,0x00,0x2e,0x00,
            0x3e,0x00,0xbc,0x01,0xb3,0x02,0x00,0x09,0x00,0x19,0x0c,0x00,0x20,0x00,0x00,0x00,
            0x80,0x00,0x00,0x0a,0x00,0x60,0x00,0x7e,0x00,0x60,0x01,0xbb,0x02,0x00,0x7f,0x00,
            0x08,0xff,0x02,0x00,0x1b,0x00,0x7e,0xff,0xff,0xff,0xff,0xff,
	    /*
	    0x00,0x01,0xac,0x00,
            0x01,0xae,0x00,0x01,0xaf,0x00,0x01,0xad,
	    */
	    0xff, 0xff, 0xff, 0xff,
	    0xff,0xff,0x00,0x00,0x2e,0xff,0x00,0x00,
            0x2a,0xff,0x00,0x00,0x2b,0xff,0x00,0x00,0x1b,0xff,0xff,0xff,0x0e,0x00,0x2f,0x00,
            0x5c,0x00,0x2f,0x00,0x1c,0x00,0x2f,0x00,0x5c,0x00,0x00,0x0a,0x00,0x00,0x00,0x0d, //XX03
            0xff,0x00,0x00,0x2d,0xff,0xff,0x0e,0x00,0x3d,0x00,0x7c,0x00,0x3d,0x00,0x1c,0x00,
            0x3d,0x00,0x7c,0x00,0x00,0x18,0x46,0x00,0x00,0x30,0x00,0x00,0x31,0x00,0x00,0x32,
            0x00,0x00,0x33,0x00,0x00,0x34,0x00,0x00,0x35,0x00,0x00,0x36,0x00,0x00,0x37,0xff,
            0x00,0x00,0x38,0x00,0x00,0x39,0xff,0xff,0xff,0x00,0xfe,0x24,0x00,0xfe,0x25,0x00,
            0xfe,0x26,0x00,0xfe,0x22,0x00,0xfe,0x27,0x00,0xfe,0x28,0xff,0x00,0xfe,0x2a,0xff,
            0x00,0xfe,0x32,0xff,0x00,0xfe,0x33,0xff,0x00,0xfe,0x29,0xff,0x00,0xfe,0x2b,0xff,
            0x00,0xfe,0x34,0xff,0x00,0xfe,0x2e,0x00,0xfe,0x30,0x00,0xfe,0x2d,0x00,0xfe,0x23,
            0x00,0xfe,0x2f,0x00,0xfe,0x21,0x00,0xfe,0x31,0x00,0xfe,0x20,
	    //A.W.  Added following 4 lines to fix wakeup on PowerBooks. 
	    0x00,0x01,0xac, //ADB=0x7b is left arrow
	    0x00,0x01,0xae, //ADB = 0x7c is right arrow
	    0x00,0x01,0xaf, //ADB=0x7d is down arrow. 
	    0x00,0x01,0xad, //ADB=0x7e is up arrow	
	    0x0f,0x02,0xff,0x04,
            0x00,0x31,0x02,0xff,0x04,0x00,0x32,0x02,0xff,0x04,0x00,0x33,0x02,0xff,0x04,0x00,
            0x34,0x02,0xff,0x04,0x00,0x35,0x02,0xff,0x04,0x00,0x36,0x02,0xff,0x04,0x00,0x37,
            0x02,0xff,0x04,0x00,0x38,0x02,0xff,0x04,0x00,0x39,0x02,0xff,0x04,0x00,0x30,0x02,
            0xff,0x04,0x00,0x2d,0x02,0xff,0x04,0x00,0x3d,0x02,0xff,0x04,0x00,0x70,0x02,0xff,
            0x04,0x00,0x5d,0x02,0xff,0x04,0x00,0x5b,
0x06, // following are 6 special keys
0x05,0x72,  //NX_KEYTYPE_HELP is 5, ADB code is 0x72
0x06,0x7f,  //NX_POWER_KEY is 6, ADB code is 0x7f
0x07,0x4a,  //NX_KEYTYPE_MUTE is 7, ADB code is 0x4a
0x08,0x3e,  //NX_UP_ARROW_KEY is 8, ADB is 3e raw, 7e virtual (KMAP)
0x09,0x3d,   //NX_DOWN_ARROW_KEY is 9, ADB is 0x3d raw, 7d virtual
0x0a,0x47   //NX_KEYTYPE_NUM_LOCK is 10, ADB combines with CLEAR key for numlock
    };

*length = sizeof(appleUSAKeyMap);
return appleUSAKeyMap;
}



