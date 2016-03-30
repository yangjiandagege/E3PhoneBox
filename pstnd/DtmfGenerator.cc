/**
 * @file DtmfGenerator.cc
 * @author shujie.li
 * @version 1.0
 */

#ifndef _DTMF_GENERATOR_CC_
#define _DTMF_GENERATOR_CC_ 1

typedef int INT32;
typedef unsigned int UINT32;
typedef short INT16;
typedef unsigned short UINT16;



// Class DtmfGenerator is used for generating of DTMF
// frequences, corresponding push buttons.

class DtmfGenerator
{
private:
	INT16 tempCoeff[8];
    INT32 countDurationPushButton;
    INT32 countDurationPause;
    INT32 tempCountDurationPushButton;
    INT32 tempCountDurationPause;
    INT32 readyFlag;
    char pushDialButtons[20];
    UINT32 countLengthDialButtonsArray;
    UINT32 count;
    INT32 sizeOfFrame;

    short tempCoeff1, tempCoeff2;
    INT32 y1_1, y1_2, y2_1, y2_2;


	inline INT32 MPY48SR(INT16 o16, INT32 o32)
	{
	    UINT32   Temp0;
	    INT32    Temp1;
	    Temp0 = (((UINT16)o32 * o16) + 0x4000) >> 15;
	    Temp1 = (INT16)(o32 >> 16) * o16;
	    return (Temp1 << 1) + Temp0;
	}

	void frequency_oscillator(INT16 Coeff0, INT16 Coeff1,
                                 INT16 y[], UINT32 COUNT,
                                 INT32 *y1_0, INT32 *y1_1,
                                 INT32 *y2_0, INT32 *y2_1)
	{
	    register INT32 Temp1_0, Temp1_1, Temp2_0, Temp2_1, Temp0, Temp1, Subject;
	    UINT16 ii;
	    Temp1_0 = *y1_0,
	    Temp1_1 = *y1_1,
	    Temp2_0 = *y2_0,
	    Temp2_1 = *y2_1,
	    Subject = Coeff0 * Coeff1;
	    for(ii = 0; ii < COUNT; ++ii)
	    {
	        Temp0 = MPY48SR(Coeff0, Temp1_0 << 1) - Temp2_0,
	        Temp1 = MPY48SR(Coeff1, Temp1_1 << 1) - Temp2_1;
	        Temp2_0 = Temp1_0,
	        Temp2_1 = Temp1_1;
	        Temp1_0 = Temp0,
	        Temp1_1 = Temp1,
	        Temp0 += Temp1;
	        if(Subject)
	            Temp0 >>= 1;
	        y[ii] = (INT16)Temp0;
	    }
	
	    *y1_0 = Temp1_0,
	     *y1_1 = Temp1_1,
	      *y2_0 = Temp2_0,
	       *y2_1 = Temp2_1;
	}

public:

    // FrameSize - Size of frame, DurationPush - duration pushed button in ms
    // DurationPause - duration pause between pushed buttons in ms
    DtmfGenerator(INT32 FrameSize, INT32 DurationPush = 70, INT32 DurationPause = 50)
    {
	    countDurationPushButton = (DurationPush << 3) / FrameSize + 1;
	    countDurationPause = (DurationPause << 3) / FrameSize + 1;
	    sizeOfFrame = FrameSize;
	    readyFlag = 1;
	    countLengthDialButtonsArray = 0;
	    tempCoeff[0] = 27980;
		tempCoeff[1] = 26956;
		tempCoeff[2] = 25701;
		tempCoeff[3] = 24218;
	    tempCoeff[4] = 19073;
		tempCoeff[5] = 16325;
		tempCoeff[6] = 13085;
		tempCoeff[7] = 9315;
	};

    ~DtmfGenerator()
    {};

    //That function will be run on each outcoming frame
    void dtmfGenerating(INT16* y)
    {
	    if(readyFlag)
	    {
	    	return;
	    }

	    while(countLengthDialButtonsArray > 0)
	    {
	        if(countDurationPushButton == tempCountDurationPushButton)
	        {
	            switch(pushDialButtons[count])
	            {
	            case '1':
	                tempCoeff1 = tempCoeff[0];
	                tempCoeff2 = tempCoeff[4];
	                y1_1 = tempCoeff[0];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[4];
	                y2_2 = 31000;
	                break;
	            case '2':
	                tempCoeff1 = tempCoeff[0];
	                tempCoeff2 = tempCoeff[5];
	                y1_1 = tempCoeff[0];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[5];
	                y2_2 = 31000;
	                break;
	            case '3':
	                tempCoeff1 = tempCoeff[0];
	                tempCoeff2 = tempCoeff[6];
	                y1_1 = tempCoeff[0];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[6];
	                y2_2 = 31000;
	                break;
	            case 'A':
	                tempCoeff1 = tempCoeff[0];
	                tempCoeff2 = tempCoeff[7];
	                y1_1 = tempCoeff[0];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[7];
	                y2_2 = 31000;
	                break;
	            case '4':
	                tempCoeff1 = tempCoeff[1];
	                tempCoeff2 = tempCoeff[4];
	                y1_1 = tempCoeff[1];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[4];
	                y2_2 = 31000;
	                break;
	            case '5':
	                tempCoeff1 = tempCoeff[1];
	                tempCoeff2 = tempCoeff[5];
	                y1_1 = tempCoeff[1];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[5];
	                y2_2 = 31000;
	                break;
	            case '6':
	                tempCoeff1 = tempCoeff[1];
	                tempCoeff2 = tempCoeff[6];
	                y1_1 = tempCoeff[1];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[6];
	                y2_2 = 31000;
	                break;
	            case 'B':
	                tempCoeff1 = tempCoeff[1];
	                tempCoeff2 = tempCoeff[7];
	                y1_1 = tempCoeff[1];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[7];
	                y2_2 = 31000;
	                break;
	            case '7':
	                tempCoeff1 = tempCoeff[2];
	                tempCoeff2 = tempCoeff[4];
	                y1_1 = tempCoeff[2];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[4];
	                y2_2 = 31000;
	                break;
	            case '8':
	                tempCoeff1 = tempCoeff[2];
	                tempCoeff2 = tempCoeff[5];
	                y1_1 = tempCoeff[2];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[5];
	                y2_2 = 31000;
	                break;
	            case '9':
	                tempCoeff1 = tempCoeff[2];
	                tempCoeff2 = tempCoeff[6];
	                y1_1 = tempCoeff[2];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[6];
	                y2_2 = 31000;
	                break;
	            case 'C':
	                tempCoeff1 = tempCoeff[2];
	                tempCoeff2 = tempCoeff[7];
	                y1_1 = tempCoeff[2];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[7];
	                y2_2 = 31000;
	                break;
	            case '*':
	                tempCoeff1 = tempCoeff[3];
	                tempCoeff2 = tempCoeff[4];
	                y1_1 = tempCoeff[3];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[4];
	                y2_2 = 31000;
	                break;
	            case '0':
	                tempCoeff1 = tempCoeff[3];
	                tempCoeff2 = tempCoeff[5];
	                y1_1 = tempCoeff[3];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[5];
	                y2_2 = 31000;
	                break;
	            case '#':
	                tempCoeff1 = tempCoeff[3];
	                tempCoeff2 = tempCoeff[6];
	                y1_1 = tempCoeff[3];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[6];
	                y2_2 = 31000;
	                break;
	            case 'D':
	                tempCoeff1 = tempCoeff[3];
	                tempCoeff2 = tempCoeff[7];
	                y1_1 = tempCoeff[3];
	                y2_1 = 31000;
	                y1_2 = tempCoeff[7];
	                y2_2 = 31000;
	                break;
	            default:
	                tempCoeff1 = tempCoeff2 = 0;
	                y1_1 = 0;
	                y2_1 = 0;
	                y1_2 = 0;
	                y2_2 = 0;
	            }
	        }
	        while(tempCountDurationPushButton > 0)
	        {
	            --tempCountDurationPushButton;
	
	            frequency_oscillator(tempCoeff1, tempCoeff2,
	                                 y, sizeOfFrame,
	                                 &y1_1, &y1_2,
	                                 &y2_1, &y2_2
	                                );
	            return;
	        }
	
	        while(tempCountDurationPause > 0)
	        {
	            --tempCountDurationPause;
	            for(INT32 ii = 0; ii < sizeOfFrame; ii++)
	            {
	                y[ii] = 0;
	            }
	            return;
	        }
	
	        tempCountDurationPushButton = countDurationPushButton;
	        tempCountDurationPause = countDurationPause;
	
	        ++count;
	        --countLengthDialButtonsArray;
	    }
	    readyFlag = 1;
	    return;
	};

    // If transmitNewDialButtonsArray return 1 then the dialButtonsArray will be transmitted
    // if 0, transmit is not possible and is needed to wait (nothing will be transmitted)
    // Warning! lengthDialButtonsArray must to be < 21 and != 0, if lengthDialButtonsArray will be > 20
    // will be transmitted only first 20 dial buttons
    // if lengthDialButtonsArray == 0 will be returned 1 and nothing will be transmitted
    INT32 transmitNewDialButtonsArray(char* dialButtonsArray, UINT32 lengthDialButtonsArray)
    {
	    if(getReadyFlag() == 0) return 0;
	    if(lengthDialButtonsArray == 0)
	    {
	        countLengthDialButtonsArray = 0;
	        count = 0;
	        readyFlag = 1;
	        return 1;
	    }
	    countLengthDialButtonsArray = lengthDialButtonsArray;
	    if(lengthDialButtonsArray > 20) countLengthDialButtonsArray = 20;
	    for(unsigned ii = 0; ii < countLengthDialButtonsArray; ii++)
	    {
	        pushDialButtons[ii] = dialButtonsArray[ii];
	    }
	
	    tempCountDurationPushButton = countDurationPushButton;
	    tempCountDurationPause = countDurationPause;
	
	    count = 0;
	    readyFlag = 0;
	    return 1;
	};

    //Reset generation
    void dtmfGeneratorReset()
    {
        countLengthDialButtonsArray = 0;
        count = 0;
        readyFlag = 1;
    }


    //If getReadyFlag return 1 then a new button's array may be transmitted
    // if 0 transmit is not possible and is needed to wait
    INT32 getReadyFlag() const
    {
        return readyFlag ? 1 : 0;
    }
};

#endif //_DTMF_GENERATOR_CC_
