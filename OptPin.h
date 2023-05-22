/* OptPin.h
 *
 * Created: 17.08.2018 22:49:41
 *  Author: Павел
 */ 

#pragma once

//#include "MyFunctions.h"

#ifdef ARDUINO

#include <Arduino.h>

// Safety checks and timer features removed
inline void OPTdigitalWrite(uint8_t pin, bool val);
// Safety checks and timer features removed
inline bool OPTdigitalRead(uint8_t pin);
//Sets ADMUX ASAP
inline void OPTanalogReference(uint8_t val);
uint16_t OPTanalogRead();

// Safety checks and timer features removed
inline void OPTdigitalWrite(uint8_t pin, bool val)
{
	uint8_t bit = digitalPinToBitMask(pin);
	volatile uint8_t *out = portOutputRegister(digitalPinToPort(pin));
	val ? *out |= bit : *out &= ~bit;
}

// Safety checks and timer features removed
inline bool OPTdigitalRead(uint8_t pin)
{
	return (*portInputRegister(digitalPinToPort(pin)) & digitalPinToBitMask(pin)) > 0;
}

//Sets ADMUX ASAP
inline void OPTanalogReference(uint8_t val)
{
	#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
	ADMUX = val << 4;
	#else
	ADMUX = val << 6;
	#endif
}

//Accepts only channels! Refer to current MCU datasheet!
inline void OPTanalogChannel(uint8_t channel)
{
	// the MUX5 bit of ADCSRB selects whether we're reading from channels
	// 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
	#if defined(ADCSRB) && defined(MUX5)
	if (channel > 7_ui8) {
		ADCSRB |= static_cast<uint8_t>(_BV(MUX5));
		channel &= 7_ui8;
	}
	#endif
	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
	ADMUX |= channel;
}

//Accepts only channels! Refer to current MCU datasheet!
uint16_t OPTanalogRead(uint8_t channel); //Probably inlining it is not a good idea

#else

#include <avr/pgmspace.h>
#include <avr/io.h>

#endif

#include <stdint.h>
#include <stddef.h>

//Templates
#ifdef ARRAYSIZE_UINT8
	template<typename T, uint8_t s> constexpr uint8_t arraySize(const T (&v)[s]);
	template<typename T, uint8_t s> constexpr uint8_t arraySize(const T (&v)[s])
	{
		return s;
	}
#else
	template<typename T, size_t s> constexpr size_t arraySize(const T (&v)[s]);
	template<typename T, size_t s> constexpr size_t arraySize(const T (&v)[s])
	{
		return s;
	}
#endif

//Operators
constexpr uint8_t operator"" _ui8(const unsigned long long int val)
{
	return static_cast<uint8_t>(val);
}

/************************************************************************/
/* Configuration section                                                */
/************************************************************************/
#define PINS_STATIC 1
#define ENABLE_EXTRA_FEATURES 0
#define PINS_STATIC_INLINE 1
using desc_t = uint8_t;
using annot_t = uint8_t*; /*PGM_P;*/
/************************************************************************/
/* End of configuration section                                         */
/************************************************************************/
#define BVD(val) static_cast<desc_t>(static_cast<desc_t>(1U) << (val))
#define BV8(val) static_cast<uint8_t>(1_ui8 << static_cast<uint8_t>(val))
/************************************************************************/
/*  Pin object                                                          */
/************************************************************************/
struct pin_t
{
	static const bool Input = false;
	static const bool Output = true;
	volatile uint8_t* const ddrReg;
	const uint8_t bitMask;
#if ENABLE_EXTRA_FEATURES
	annot_t Text;
#endif
	pin_t(volatile uint8_t&, const uint8_t
#if ENABLE_EXTRA_FEATURES	
	, annot_t 
#endif
	);
#if PINS_STATIC
	#if PINS_STATIC_INLINE
		static inline void Write(const pin_t*, bool);
		static inline bool Read(const pin_t*);
		static inline void SetDirection(const pin_t*, bool);
	#else
		static void Write(const pin_t*, bool);
		static bool Read(const pin_t*);
		static void SetDirection(const pin_t*, bool);
	#endif
#else
	void Write(bool val) const
	{
		val ? ( *(this->ddrReg + 1U) |= this->bitMask ) : ( *(this->ddrReg + 1U) &= static_cast<uint8_t>(~(this->bitMask)) );
	}
	bool Read() const
	{
		return static_cast<uint8_t>(*(this->ddrReg - 1U) & this->bitMask) != 0_ui8;
	}
	void SetDirection(bool dir) const
	{
		dir ? ( *(this->ddrReg) |= this->bitMask ) : ( *(this->ddrReg) &= static_cast<uint8_t>(~(this->bitMask)) );
	}
#endif
#if ENABLE_EXTRA_FEATURES
	enum Descriptor : uint8_t
	{
		Inversion,
		NormalState,
		ManualOverride
	};
	#if PINS_STATIC
		#if PINS_STATIC_INLINE
		static inline void SetDescriptor(pin_t*, uint8_t, bool);
		static inline bool ReadDescriptor(const pin_t*, uint8_t);
		#else
		static void SetDescriptor(pin_t*, uint8_t, bool);
		static bool ReadDescriptor(const pin_t*, uint8_t);
		#endif
	#else
	void SetDescriptor(Descriptor bit, bool val)
	{
		val ? ( this->desc |= BVD(bit) ) : ( this->desc &= static_cast<desc_t>(~(BVD(bit))) );
	}
	bool ReadDescriptor(Descriptor bit)
	{
		return static_cast<desc_t>(this->desc & BVD(bit)) != static_cast<desc_t>(0U);
	}
	#endif
#endif
protected:
#if ENABLE_EXTRA_FEATURES
	desc_t desc;	
#endif
};
#if PINS_STATIC
	#if PINS_STATIC_INLINE
	inline void pin_t::Write(const pin_t* pin, bool val)
	{
		val ? ( *(pin->ddrReg + 1U) |= pin->bitMask ) : ( *(pin->ddrReg + 1U) &= static_cast<uint8_t>(~(pin->bitMask)) );
	}
	inline bool pin_t::Read(const pin_t* pin)
	{
		return static_cast<uint8_t>(*(pin->ddrReg - 1U) & pin->bitMask) != 0_ui8;
	}
	inline void pin_t::SetDirection(const pin_t* pin, bool dir)
	{
		dir ? ( *(pin->ddrReg) |= pin->bitMask ) : ( *(pin->ddrReg) &= static_cast<uint8_t>(~(pin->bitMask)) );
	}
	#endif
	#if ENABLE_EXTRA_FEATURES
		#if PINS_STATIC_INLINE
		inline void pin_t::SetDescriptor(pin_t* pin, uint8_t bit, bool val)
		{
			val ? ( pin->desc |= BVD(bit) ) : ( pin->desc &= static_cast<desc_t>(~(BVD(bit))) );
		}
		inline bool pin_t::ReadDescriptor(const pin_t* pin, uint8_t bit)
		{
			return static_cast<desc_t>(pin->desc & BVD(bit)) != static_cast<desc_t>(0U);
		}
		#endif
	#endif
#endif