//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief Interface for Yamaha YM2151

#pragma once

#include <cstring>

#include "Chip.h"

#undef  DBG
#define DBG if (0) printf

namespace YM2151 {

enum Param
{
   // Params
   TEST,
   NOISE_FRQ, NOISE_EN,
   TIMER_A,TIMER_B,
   TIMER_LOAD, TIMER_LOAD_A,TIMER_LOAD_B,
   TIMER_RST, TIMER_RST_A,TIMER_RST_B,
   TIMER_IRQ, TIMER_IRQ_A,TIMER_IRQ_B,
   TIMER_CSM,
   LFO_FRQ, LFO_AMD, LFO_PMD, LFO_WAVE,
   CT, CT1, CT2,

   // Channel params
   CONECT, FB, RL, KC, KF, AMS, PMS,

   // Op params
   MUL, DT1, DT2,
   EG_TL, EG_AR, EG_D1R, EG_D2R, EG_RR, EG_D1L,
   AMS_EN, KS
};

static const uint8_t  OP_M1  = 0b0001;
static const uint8_t  OP_C1  = 0b0010;
static const uint8_t  OP_M2  = 0b0100;
static const uint8_t  OP_C2  = 0b1000;
static const uint8_t  OP_ALL = OP_M1 | OP_C1 | OP_M2 | OP_C2;
static const unsigned NUM_OP = 4;

class Interface : public Chip
{
public:
   Interface()
      : Chip("YM2151", /* num_voices */ 8, /* clock_ticks_per_sample */ 64)
   {
   }

//------------------------------------------------------------------------------
// Implement MIDI::Instrument

   //! Play a note
   void voiceOn(unsigned voice_, uint8_t midi_note_, uint8_t velocity_) override
   {
      static const unsigned table[12] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14};

      midi_note_ -= 1;

      unsigned octave = midi_note_ / 12;
      unsigned note   = table[midi_note_ % 12];

      setCh<KC>(voice_, (octave << 4) | note);

      writeReg(0x08, (OP_ALL << 3) | voice_);
   }

   //! Stop a note
   void voiceOff(unsigned voice_, uint8_t velocity_) override
   {
      writeReg(0x08, voice_);
   }

   //! Set channel (0-3) attenuation
   void voicePressure(unsigned voice_, uint8_t pressure_) override
   {
   }

   virtual void voicePitchBend(unsigned voice_, int16_t value_) override
   {
   }

//------------------------------------------------------------------------------

   //! 
   virtual void hardReset()
   {
      memset(shadow, 0, sizeof(shadow));

      softReset();
   }

   //! Initialize registers to stop all sounds and timer activity
   void softReset()
   {
      set<NOISE_EN>(0);   // Disable noise

      set<TIMER_LOAD>(0); // Stop timers
      set<TIMER_IRQ>(0);  // Disable interrupts
      set<TIMER_CSM>(0);  // Clear CSM

      for(unsigned voice = 0; voice < getNumVoices(); voice++)
      {
         voiceOff(voice, 0);

         // Config channel operators
         for(uint8_t op = OP_M1; op < OP_ALL; op <<= 1)
         {
            setOp<EG_AR>( voice, op, 31);
            setOp<EG_D1R>(voice, op, 0);
            setOp<EG_D1L>(voice, op, 0);
            setOp<EG_D2R>(voice, op, 0);
            setOp<EG_RR>( voice, op, 15);

            setOp<EG_TL>( voice, op, 10);
            setOp<MUL>(   voice, op, 1);
         }

         // Config voice
         setCh<CONECT>(voice, 7);
         setCh<FB>(    voice, 0);
         setCh<RL>(    voice, 0b11);

         setCh<KF>(    voice, 0);
         setCh<AMS>(   voice, 0);
         setCh<PMS>(   voice, 0);
      }
   }

   //! Set parameter
   template <YM2151::Param PARAM>
   void set(unsigned value_)
   {
      switch(PARAM)
      {
      case TEST:         writeReg(  0x01,       value_); break;

      case NOISE_FRQ:    writeField(0x0F, 9, 5, value_); break;
      case NOISE_EN:     writeField(0x0F, 7, 1, value_); break;

      case TIMER_A:      writeReg(  0x10,       value_ >> 2);
                         writeField(0x11, 0, 2, value_); break;
      case TIMER_B:      writeReg(  0x12,       value_); break;
      case TIMER_LOAD:   writeField(0x14, 0, 2, value_); break;
      case TIMER_LOAD_A: writeField(0x14, 0, 1, value_); break;
      case TIMER_LOAD_B: writeField(0x14, 1, 1, value_); break;
      case TIMER_RST:    writeField(0x14, 4, 2, value_); break;
      case TIMER_RST_A:  writeField(0x14, 4, 1, value_); break;
      case TIMER_RST_B:  writeField(0x14, 5, 1, value_); break;
      case TIMER_IRQ:    writeField(0x14, 2, 2, value_); break;
      case TIMER_IRQ_A:  writeField(0x14, 2, 1, value_); break;
      case TIMER_IRQ_B:  writeField(0x14, 3, 1, value_); break;
      case TIMER_CSM:    writeField(0x14, 7, 1, value_); break;

      case LFO_FRQ:      writeReg(  0x18,       value_); break;
      case LFO_AMD:      writeReg(  0x19,       value_ & 0x7F); break;
      case LFO_PMD:      writeReg(  0x19,       value_ | 0x80); break;
      case LFO_WAVE:     writeField(0x1B, 0, 2, value_); break;

      case CT:           writeField(0x1B, 6, 2, value_); break;
      case CT2:          writeField(0x1B, 7, 1, value_); break;
      case CT1:          writeField(0x1B, 6, 1, value_); break;

      default: break;
      }
   }

   //! Set channel parameter
   template <YM2151::Param PARAM>
   void setCh(unsigned channel_, unsigned value_)
   {
      switch(PARAM)
      {
      case CONECT: writeField(0x20 + channel_, 0, 3, value_); break;
      case FB:     writeField(0x20 + channel_, 3, 3, value_); break;
      case RL:     writeField(0x20 + channel_, 6, 2, value_); break;
      case KC:     writeField(0x28 + channel_, 0, 7, value_); break;
      case KF:     writeField(0x30 + channel_, 1, 7, value_); break;
      case AMS:    writeField(0x38 + channel_, 0, 2, value_); break;
      case PMS:    writeField(0x38 + channel_, 4, 3, value_); break;

      default: break;
      }
   }

   //! Set operator parameter
   template <YM2151::Param PARAM>
   void setOp(unsigned channel_, uint8_t op_, unsigned value_)
   {
      unsigned offset;

      switch(op_)
      {
      case OP_M1: offset =      channel_; break;
      case OP_M2: offset =  8 + channel_; break;
      case OP_C1: offset = 16 + channel_; break;
      case OP_C2: offset = 24 + channel_; break;
      }

      switch(PARAM)
      {
      case DT1:    writeField(0x40 + offset, 4, 3, value_); break;
      case MUL:    writeField(0x40 + offset, 0, 4, value_); break;
      case EG_TL:  writeField(0x60 + offset, 0, 7, value_); break;
      case KS:     writeField(0x80 + offset, 6, 2, value_); break;
      case EG_AR:  writeField(0x80 + offset, 0, 5, value_); break;
      case AMS_EN: writeField(0xA0 + offset, 7, 1, value_); break;
      case EG_D1R: writeField(0xA0 + offset, 0, 5, value_); break;
      case DT2:    writeField(0xC0 + offset, 6, 2, value_); break;
      case EG_D2R: writeField(0xC0 + offset, 0, 5, value_); break;
      case EG_D1L: writeField(0xE0 + offset, 4, 4, value_); break;
      case EG_RR:  writeField(0xE0 + offset, 0, 4, value_); break;

      default: break;
      }
   }

   //! Write a register
   void writeReg(uint8_t addr_, uint8_t data_)
   {
      if ((addr_ >= 0x20) && (data_ == shadow[addr_]))
         return;

      writeBus(A0_ADDR, addr_);
      writeBus(A0_DATA, data_);

      waitForReady();

      shadow[addr_] = data_;

      DBG("WR %02X => %02X\n", data_, addr_);
   }

   //! Read a register (expected value not actual)
   uint8_t readReg(uint8_t addr_) const
   {
      return shadow[addr_];
   }

   void reset() override
   {
      for(unsigned v = 0; v < getNumVoices(); v++)
      {
         voiceOff(v, 0);
      }
   }

   void write(uint16_t addr_, uint8_t data_) override
   {
      writeReg(addr_, data_);
   }

protected:
   //! Write a field in a register
   void writeField(uint8_t  addr_,
                   unsigned ls_bit_,
                   unsigned bits_,
                   unsigned data_)
   {
      uint8_t data_mask = (1 << bits_) - 1;
      uint8_t reg_mask  = ~(data_mask << ls_bit_);

      uint8_t data = (shadow[addr_] & reg_mask) |
                     (data_ & data_mask) << ls_bit_;

      writeReg(addr_, data);
   }

   //! Write a byte to the YM2151 bus
   virtual void writeBus(bool a0_, uint8_t value_) = 0;

   //! Wait for YM2151 bus ready
   virtual void waitForReady() = 0;

   static constexpr bool A0_ADDR = false;
   static constexpr bool A0_DATA = true;

private:
   uint8_t shadow[256];
};

} // namespace YM2151
