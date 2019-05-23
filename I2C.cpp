/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#include "I2C.h"
bool Initialized = false;

I2C::I2C() {}

void I2C::init(uint8_t address) {
    this->address = address;
    if(Initialized ==false)
    {
      TWSR = 0;
      TWBR = ((F_CPU/SCL_CLOCK)-16)/2;
      Initialized = true;
    }
}

uint8_t I2C::start() {
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)));

    twi_status_register = TW_STATUS & 0xF8;
    if ((this->twi_status_register != TW_START) && (this->twi_status_register != TW_REP_START)) {
        return 1;
    }

    TWDR = address&0xFE;
    TWCR = (1<<TWINT) | (1<<TWEN);

    while(!(TWCR & (1<<TWINT)));

    this->twi_status_register = TW_STATUS & 0xF8;
    if ((this->twi_status_register != TW_MT_SLA_ACK) && (this->twi_status_register != TW_MR_SLA_ACK)) {
        return 1;
    }

    return 0;
}
uint8_t I2C::startRead(bool Ack) {
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)));

    twi_status_register = TW_STATUS & 0xF8;
    if ((this->twi_status_register != TW_START) && (this->twi_status_register != TW_REP_START)) {
        return 1;
    }

    TWDR = address|0x01;
    if(Ack == false)
    {
      TWCR = (1<<TWINT) | (1<<TWEN);
    }
    else
    {
      TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    }

    while(!(TWCR & (1<<TWINT)));

    this->twi_status_register = TW_STATUS & 0xF8;
    if ((this->twi_status_register != TW_MT_SLA_ACK) && (this->twi_status_register != TW_MR_SLA_ACK)) {
        return 1;
    }

    return 0;
}

uint8_t I2C::write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWINT) | (1<<TWEN);

    while(!(TWCR & (1<<TWINT)));

    this->twi_status_register = TW_STATUS & 0xF8;
    if (this->twi_status_register != TW_MT_DATA_ACK) {
        return 1;
    } else {
        return 0;
    }
}
uint8_t I2C::read(bool Ack)
{
    //TWDR = 0x00;
    if(Ack)
      TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    else
      TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1 << TWINT)));

    return TWDR;
}

void I2C::stop(void) {
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    while(TWCR & (1<<TWSTO));
}
void I2C::writeAddress(uint8_t addr, uint8_t value)
{
  this->start();
  this->write(addr);
  this->write(value);
  this->stop();
}
void I2C::writeAddress(uint8_t addr, uint8_t *value, uint8_t length)
{
  this->start();
  this->write(addr);
  for(uint8_t i=0; i<length; ++i)
  {
      this->write(value[i]);
  }
  this->stop();
}
uint8_t I2C::readAddress(uint8_t addr)
{
  this->start();
  this->write(addr);
  //BMA_I2C.stop();
  this->startRead(false);
  uint8_t data = this->read(false);
  this->stop();

  return data;
}
uint8_t I2C::readAddress(uint8_t addr, uint8_t length, uint8_t *data)
{
  this->start();
  this->write(addr);
  this->stop();
  this->startRead(true);
  for(uint8_t i=0;i<length-1;++i)
     data[i] = this->read(true);
  data[length-1] = this->read(false);
  this->stop();

  return data[length-1];
}
