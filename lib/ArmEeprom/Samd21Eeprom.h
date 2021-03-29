/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


  EEPROM like API that uses Arduino Zero's flash memory. 
  Requirements: cmaglie's Flash Storage: https://github.com/cmaglie/FlashStorage
  (c) 2016, A. Christian
*/

#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32F2)

#ifndef EEPROM_h
#define EEPROM_h

#include <FlashStorage.h>

#ifndef EEPROM_EMULATION_SIZE
#define EEPROM_EMULATION_SIZE 1024
#endif

typedef struct {
  byte data[EEPROM_EMULATION_SIZE];
  boolean valid;  
} EEPROM_EMULATION;


class EEPROMClass {

  public:
    EEPROMClass(void);
    uint8_t read(int);
    void write(int, uint8_t);
    void update(int, uint8_t);
    bool isValid();
    void init();
    void commit();

  protected:
    EEPROM_EMULATION _eeprom;
    bool _dirty;
    FlashStorageClass<EEPROM_EMULATION> *_flash;

};

extern EEPROMClass EEPROM;

#endif

#endif
