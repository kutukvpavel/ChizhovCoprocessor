/*
 * OptPin.cpp
 *
 * Created: 11.11.2018 5:31:44
 *  Author: Павел
 */ 

#include "OptPin.h"

uint16_t OPTanalogRead() //Probably inlining it is not a good idea
{
	// start the conversion
	ADCSRA |= static_cast<uint8_t>(_BV(ADSC)); //No more obsolete definitions like SBI which when expanded are an unnecessary nightmare
	// ADSC is cleared when the conversion finishes
	while (ADCSRA & static_cast<uint8_t>(_BV(ADSC)));
	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	uint8_t low, high;
	low = ADCL;
	high = ADCH;
	// combine the two bytes
	return (high << 8) | low;
}

pin_t::pin_t(volatile uint8_t& ddr_reg, const uint8_t mask
#if ENABLE_EXTRA_FEATURES
, annot_t txt
#endif
) : ddrReg(&ddr_reg), bitMask(BV8(mask))
#if ENABLE_EXTRA_FEATURES
, Text(txt)
#endif
{};

#if PINS_STATIC
	#if !PINS_STATIC_INLINE
	void pin_t::Write(const pin_t* pin, bool val)
	{
		val ? ( *(pin->ddrReg + 1U) |= pin->bitMask ) : ( *(pin->ddrReg + 1U) &= static_cast<uint8_t>(~(pin->bitMask)) );
	}
	bool pin_t::Read(const pin_t* pin)
	{
		return static_cast<uint8_t>(*(pin->ddrReg - 1U) & pin->bitMask) != 0_ui8;
	}
	void pin_t::SetDirection(const pin_t* pin, bool dir)
	{
		dir ? ( *(pin->ddrReg) |= pin->bitMask ) : ( *(pin->ddrReg) &= static_cast<uint8_t>(~(pin->bitMask)) );
	}
	#endif
	#if ENABLE_EXTRA_FEATURES
		#if !PINS_STATIC_INLINE
		void pin_t::SetDescriptor(pin_t* pin, uint8_t bit, bool val)
		{
			val ? ( pin->desc |= BVD(bit) ) : ( pin->desc &= static_cast<desc_t>(~(BVD(bit))) );
		}
		bool pin_t::ReadDescriptor(const pin_t* pin, uint8_t bit)
		{
			return static_cast<desc_t>(pin->desc & BVD(bit)) != static_cast<desc_t>(0U);
		}
		#endif
	#endif
#endif