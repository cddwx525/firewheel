uint8_t i2cWrite(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t size
        , bool is_sendStop)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(data, size);
    return Wire.endTransmission(is_sendStop);
}

uint8_t i2cRead(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t size)
{
    uint8_t rcode;
    uint8_t i;
    uint8_t n;

    Wire.beginTransmission(addr);
    Wire.write(reg);
    rcode = Wire.endTransmission(false);

    if (rcode != 0)
            return rcode;

    n = Wire.requestFrom(addr, size, (uint8_t)true);

    if (n != size)
            return 5;

    for (i = 0; i < size; i ++)
            data[i] = Wire.read();

    return 0;
}
