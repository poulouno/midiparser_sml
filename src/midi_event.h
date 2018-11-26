#ifndef MIDIEVENT_H_
#define MIDIEVENT_H_

#include <stdint.h>

///
/// Hold a simple MIDI event (no sysex!)
class MidiEvent {
public:
	uint8_t mStatus;
	int8_t mData1;
	int8_t mData2;
	int8_t mPadding;

	/**
	 * Construct an event using from 1 up to 3 bytes.
	 * @param inStatus mandatory status byte
	 * @param inData1 1st data byte (optional)
	 * @param inData2 2nd data byte (optional)
	 */
	MidiEvent(uint8_t inStatus= kMidiReset, int8_t inData1 = 0, int8_t inData2 = 0) :
		mStatus(inStatus), mData1(inData1), mData2(inData2), mPadding(0) {}

	///
	/// Returns the event length in byte (from 1 to 3)
    constexpr static char midilen[16]={
        1,
        1,
        1,
        1,
        1,

    };
	static int getLength( uint8_t inStatus){
		if (inStatus < 0xF0) {
			switch (inStatus & 0xF0) {
			case kMidiNoteOff:
			case kMidiNoteOn:
			case kMidiPolyPress:
			case kMidiControlChange:
			case kMidiPitchBend:
				return 3;
			case kMidiProgramChange:
			case kMidiChannelPress:
				return 2;
			}
			return 2;
		}
		else if (inStatus < 0xF8) {
			switch (inStatus) {
				// System Common Messages
			case kMidiSongPositionPointer:
				return 3;
			case kMidiMTCQuarterFrame:
			case kMidiSongSelect:
				return 2;
			default:
				return 1;
			}
		} 
		else {
			// Clock releated messages
			return 1;
		}
	}
    int getLength() const {
        return getLength(mStatus);
    }
    uint8_t getChannel() const{
        return mStatus & 0x0F;
    }

    void setChannel(uint8_t inChan) {
        mStatus&= 0xF0;
        mStatus|= inChan & 0x0F;
    }
    
    static MidiEvent makeNoteOff(uint8_t inChan, int8_t inNote, int8_t inVelocity) {
        return MidiEvent(kMidiNoteOff | (inChan & 0x0F), inNote, inVelocity);
    }
    
    static MidiEvent makeNoteOn(uint8_t inChan, int8_t inNote, int8_t inVelocity) {
        return MidiEvent(kMidiNoteOn | (inChan & 0x0F), inNote, inVelocity);
    }
    
    static MidiEvent makePolyPress(uint8_t inChan, int8_t inNote, int8_t inPress) {
        return MidiEvent(kMidiPolyPress | (inChan & 0x0F), inNote, inPress);
    }
    
    static MidiEvent makeControlChange(uint8_t inChan, int8_t inCtrlNum, int8_t inCtrlVal) {
        return MidiEvent(kMidiControlChange | (inChan & 0x0F), inCtrlNum, inCtrlVal);
    }
    
    static MidiEvent makeProgramChange(uint8_t inChan, int8_t inProgram) {
        return MidiEvent(kMidiProgramChange | (inChan & 0x0F), inProgram);
    }
    
    static MidiEvent makeChannelPress(uint8_t inChan, int8_t inPress) {
        return MidiEvent(kMidiChannelPress | (inChan & 0x0F), inPress);
    }
    
    static MidiEvent makePitchBend(uint8_t inChan, int8_t inLSB, int8_t inMSB) {
        return MidiEvent(kMidiPitchBend | (inChan & 0x0F), inLSB, inMSB);
    }
    static MidiEvent makeAllNotesOff(uint8_t inChan) {
        return MidiEvent(kMidiControlChange | (inChan & 0x0F), kCCAllNotesOff, 0x0);
    }
        
    bool isNoteOff() const {
        return ((mStatus & 0xF0)== kMidiNoteOff) || ((mStatus & 0xF0)== kMidiNoteOn && mData2== 0);
    }

    bool isNoteOn() const {
        return (mStatus & 0xF0)== kMidiNoteOn && mData2> 0;
    }

    bool isPolyPress() const {
        return (mStatus & 0xF0)== kMidiPolyPress;
    }

    bool isChannelPress() const {
        return (mStatus & 0xF0)== kMidiChannelPress;
    }

    bool isPitchBend() const {
        return (mStatus & 0xF0)== kMidiPitchBend;
    }

	bool isCC() const {
        return (mStatus & 0xF0)== kMidiControlChange;
    }

    bool isVoiceMsg() const {
        return(mStatus < 0xF0);
    }
    bool isRTMsg() const {
        return(mStatus >= 0xF8);
    }
	
	enum eEventType:uint8_t {
		// Channel Voice Messages
		kMidiNoteOff = 0x80,
		kMidiNoteOn = 0x90,
		kMidiPolyPress = 0xA0,
		kMidiControlChange = 0xB0,
		kMidiProgramChange = 0xC0,
		kMidiChannelPress = 0xD0,
		kMidiPitchBend = 0xE0,
		// System Common Messages
		kMidiSysEx= 0xF0,
		kMidiMTCQuarterFrame = 0xF1,
		kMidiSongPositionPointer = 0xF2,
		kMidiSongSelect = 0xF3,
		kMidiUndefined1 = 0xF4,
		kMidiUndefined2 = 0xF5,
		kMidiTuneRequest = 0xF6,
		kMidiSysExEnd = 0xF7,
		// System Real - Time Messages
		kMidiClock = 0xF8,
		kMidiUndefined3 = 0xF9,
		kMidiStart = 0xFA,
		kMidiContinue = 0xFB,
		kMidiStop = 0xFC,
		kMidiUndefined4 = 0xFD,
		kMidiActiveSensing = 0xFE,
		kMidiReset = 0xFF
	};
    
    enum eEventChannel:uint8_t {
        kChan1= 0,
        kChan2,
        kChan3,
        kChan4,
        kChan5,
        kChan6,
        kChan7,
        kChan8,
        kChan9,
        kChan10,
        kChan11,
        kChan12,
        kChan13,
        kChan14,
        kChan15,
        kChan16
    };
    
    enum eCCNum:uint8_t {
        kCCBankMSB= 0,
        kCCMod= 1,
        kCCDataEntryMSB= 6,
        kCCBankLSB= 32,
        kCCDataEntryLSB= 38,
        kCCNRpnLSB= 98,
        kCCNRpnMSB= 99,
        kCCRpnLSB= 100,
        kCCRpnMSB= 101,
        kCCAllNotesOff= 123
    };
};

/// MidiEvent variant for NRPN and MMC
class MidiEventEx: public MidiEvent {
public:
    enum eExEventType:uint8_t {
        kMidiPressure= 0x20,
        kMidiRelCtrl= 0x30,
        kMidiPatch= 0x40,
        kMidiRPN= 0x50,
        kMidiNRPN= 0x60,
        kMidiMMC= 0x70
    };

	/**
	 * Construct an event using from 1 up to 4 bytes.
	 * @param inStatus mandatory status byte
	 * @param inData1 1st data byte (optional)
	 * @param inData2 2nd data byte (optional)
	 * @param inData3 3rd data byte (optional)
	 */
	MidiEventEx(uint8_t inStatus= kMidiReset, int8_t inData1 = 0, int8_t inData2 = 0, int8_t inData3 = 0) :
		MidiEvent(inStatus, inData1, inData2) { mPadding= inData3; }
	/**
	 * Construct an event using a MidiEvent.
	 * @param inEvent a simple (unextended) MidiEvent
	 */
	//MidiEventEx(MidiEvent inEvent) :
	//	MidiEvent(inEvent) {}

    static MidiEventEx makePressure(uint8_t inChan, uint8_t inNoteNum, int8_t inPolyPress, int8_t inChanPress) {
        return MidiEventEx(kMidiPressure | (inChan & 0x0F), inNoteNum, inPolyPress, inChanPress);
    }

    static MidiEventEx makeRelCtrl(uint8_t inChan, uint8_t inCtrlNum, int8_t inRelVal, int8_t inRel0) {
        return MidiEventEx(kMidiRelCtrl | (inChan & 0x0F), inCtrlNum, inRelVal, inRel0);
    }

    static MidiEventEx makePatch(uint8_t inChan, uint8_t inLSB, int8_t inMSB, int8_t inValue) {
        return MidiEventEx(kMidiPatch | (inChan & 0x0F), inLSB, inMSB, inValue);
    }

    static MidiEventEx makeRPN(uint8_t inChan, uint8_t inLSB, int8_t inMSB, int8_t inValue) {
        return MidiEventEx(kMidiRPN | (inChan & 0x0F), inLSB, inMSB, inValue);
    }
    
    static MidiEventEx makeNRPN(uint8_t inChan, uint8_t inLSB, int8_t inMSB, int8_t inValue) {
        return MidiEventEx(kMidiNRPN | (inChan & 0x0F), inLSB, inMSB, inValue);
    }

    static MidiEventEx makeMMC(uint8_t inChan, int8_t inValue) {
        return MidiEventEx(kMidiMMC, inChan, inValue);
    }
};

///
/// Hold Midi/sysex error code
class MidiError {
public:
	enum eMidiError:uint8_t {
		kErrNone,
		kErrNotArturia,
		kErrWrongID,
		kErrUnsupProtocol,
		kErrUnsupCommand,
		kErrUnknowItem,
		kErrFraming
	};

	MidiError() : mError(kErrNone) {}
	MidiError(int inError) : mError(static_cast<eMidiError>(inError)) {}

	eMidiError mError;
};

class MidiByte{
public: 
    
    explicit MidiByte(uint8_t b): mByte(b){};
    bool isRT() const {return mByte >= 0xF8;}
    bool isData()const{return mByte < 0x80;}
    bool isStatus()const{return !isData();}
    bool isCommon()const{return mByte < 0xF0 && !isData();}
    bool isSyxexStart()const{return mByte == 0xF0;}
    bool isSyxexEnd()const{return mByte == 0xF7;}


    uint8_t mByte;
    
};
#endif // MIDIEVENT_H_