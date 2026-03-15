//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief Common interface for synth chips

#pragma once

#include <cstdint>

#include "MIDI/Const.h"
#include "MIDI/Instrument.h"

#include "Table_midi_vol.h"
#include "Table_midi_pan.h"

#include "Sample.h"

#undef  DBG
#define DBG if (0) printf

class Chip : public MIDI::Instrument
{
public:
   Chip(const char* name_,
        unsigned num_voices_,
        unsigned clock_ticks_per_sample_)
      : MIDI::Instrument(num_voices_, /* base_channel */ 0)
      , clock_ticks_per_sample(clock_ticks_per_sample_)
      , name(name_)
   {
      controlUpdate();
   }

   const char* getName() const { return name; }

   //! Get configured sample frequency (Hz)
   unsigned getSampleFreq() const
   {
      return sample_mul * clock_freq_hz / clock_ticks_per_sample;
   }

   //! Check synth muted status
   bool isMute() const { return mute; }

   //! Set multiplier for sample frequency
   void setSampleMul(unsigned n_)
   {
      sample_mul = n_;
   }

   //! Set clock frequency (Hz)
   virtual bool setClock(unsigned clock_freq_hz_)
   {
      clock_freq_hz = clock_freq_hz_;
      mute          = clock_freq_hz_ == 0;

      return not mute;
   }

   //! Override to implement special reset behaviour
   virtual void reset() {}

   //! Override to implement special configuration
   virtual void config(unsigned param1_, unsigned param2_ = 0, unsigned param_3_ = 0) {}

   virtual unsigned addSample(uint32_t addr_, const uint8_t* ptr_, unsigned size_) { return 0; }

   //! Override to implement sound driver
   virtual void write(uint16_t addr_, uint8_t data_) = 0;

protected:
   //! Common audio processing for mono synth chips
   void mixer(int16_t mono_, Sample& mix_)
   {
      mix_ += gain * mono_;
   }

   //! Common audio processing for stero synth chips
   void mixer(const Sample& stereo_, Sample& mix_)
   {
      mix_ += gain * stereo_;
   }

   //! Control a voice
   void voiceControl(unsigned voice_, uint8_t control_, uint8_t value_) override
   {
      if (voice_ != 0)
         return;

      switch(control_)
      {
      case MIDI::CTRL_VOLUME:
         midi_vol = value_;
         controlUpdate();
         break;

      case MIDI::CTRL_PAN:
         midi_pan = value_;
         controlUpdate();
         break;

      default:
         break;
      }
   }

   volatile bool mute{true};
   unsigned      clock_freq_hz{};
   unsigned      clock_ticks_per_sample{1};

private:
   void controlUpdate()
   {
      gain.left  = table_midi_vol[midi_vol] * table_midi_pan[127 - midi_pan] >> 14;
      gain.right = table_midi_vol[midi_vol] * table_midi_pan[midi_pan] >> 14;
   }

   const char*     name{};
   uint8_t         midi_vol{127};
   uint8_t         midi_pan{64};
   volatile Sample gain{};
   unsigned        sample_mul{1};
};
