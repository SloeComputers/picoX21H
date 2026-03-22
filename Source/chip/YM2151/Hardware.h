//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief Yamaha YM2151

#pragma once

#if defined(HW_YM2151)
#include "YM2151/Interface.h"
#include "HWR/picoX21H/Config.h"
#else
#include "YM2151/Emulator.h"
#endif

namespace YM2151 {

#if defined(HW_YM2151)

class Hardware : public Interface
{
public:
   Hardware() = default;

   void start(unsigned clock_hz_)
   {
      hw.init(clock_hz_,
              HW_YM2151_CLK,
              HW_YM2151_DAC_IN);
   }

   bool setClock(unsigned clock_freq_hz_) override
   {
      hw.setClock(clock_freq_hz_);

      return Chip::setClock(clock_freq_hz_);
   }

   void hardReset() override
   {
      hw.hardReset();

      Interface::hardReset();
   }

   void writeBus(bool a0_, uint8_t value_) override
   {
      hw.writeBus(a0_, value_);
   }

   void waitForReady() override
   {
      hw.waitForReady();
   }

   void mixOut(Sample& mix_)
   {
      if (mute) return;

      int16_t left, right;
      hw.popSample(left, right);

      Sample out{left, right};
      mixer(out, mix_);
   }

private:
   ::HWR::YM2151<> hw;
};

#else

using Hardware = Emulator;

#endif

} // namespace YM2151
