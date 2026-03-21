//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief DX21 hybrid simulation for Raspberry Pi Pico

#include <cstdio>

#include "HWR/picoX21H/Config.h"
#include "HWR/FilePortal.h"

#include "Dac.h"
#include "DX21/DX21Synth.h"
#include "SynthIO.h"

// -----------------------------------------------------------------------------

static const bool MIDI_DEBUG = false;

static hw::FilePortal file_portal{"picoX21H",
                                  "https://github.com/SloeComputers/picoX21H/"};
static SynthIO     synth_io{};
static DX21::Synth dx21_synth{synth_io};


// --- Audio out DAC -----------------------------------------------------------

static chip::DacImpl<hw::Dac> dac{62500};


// --- Physical MIDI -----------------------------------------------------------

static hw::PhysMidi phys_midi{};


// --- USB MIDI ----------------------------------------------------------------

static hw::UsbFileMidi usb{0x91C0, "picoX21H", file_portal};

extern "C" void IRQ_USBCTRL() { usb.irq(); }


// --- 16x2 LCD display --------------------------------------------------------

static hw::Lcd lcd{};


void SynthIO::displayLCD(unsigned row, const char* text)
{
   lcd.move(0, row);
   lcd.print(text);
}


// --- LED ---------------------------------------------------------------------

static hw::Led led{};


// -----------------------------------------------------------------------------

#if not defined(HW_NATIVE)
static void runDAC()
{
   while(true)
   {
      Sample mix;

      dx21_synth.ym2151.mixOut(mix);
      dx21_synth.audio.process(mix);

      dac.push(mix);
   }
}
#endif

void startAudio()
{
   unsigned clock_hz       = 3579545;
   unsigned sample_rate_hz = clock_hz / 64;

   dx21_synth.start(clock_hz);
   dac.setSampleRate(sample_rate_hz);

#if not defined(HW_NATIVE)
   MTL_start_core(1, runDAC);
#endif
}

// --- Entry point -------------------------------------------------------------

int main()
{
   // Clear screen and cursor to home
   printf("\033[2J\033[H");

   printf("\n");

   puts(file_portal.addREADME("picoX21H"));

   startAudio();

   usb.setDebug(MIDI_DEBUG);
   usb.attachInstrument(1, dx21_synth);

   phys_midi.setDebug(MIDI_DEBUG);
   phys_midi.attachInstrument(1, dx21_synth);

   while(true)
   {
      phys_midi.tick();

      usb.tick();

      led = dx21_synth.isAnyVoiceOn();
   }

   return 0;
}
