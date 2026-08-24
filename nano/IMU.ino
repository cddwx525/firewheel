void IMU_init(void)
{
    i2cData[0] = 7;
    i2cData[1] = 0x00;
    i2cData[2] = 0x00;
    i2cData[3] = 0x00;
    if (i2cWrite(IMU_ADDR, 0x19, i2cData, 4, false) != 0)
    {
        beep_ms(1, 200, 0);
        while(1);
    }

    i2cData[0] = 0x01;
    if (i2cWrite(IMU_ADDR, 0x6B, i2cData, 1, true) != 0)
    {
        beep_ms(2, 200, 50);
        while(1);
    }

    if (i2cRead(IMU_ADDR, 0x75, i2cData, 1) != 0)
    {
        beep_ms(2, 200, 100);
        while(1);
    }

    if (i2cData[0] != IMU_ADDR)
    {
        beep_ms(3, 200, 50);
        while (1);
    }

    delay(100);
    if (i2cRead(IMU_ADDR, 0x3B, i2cData, 6) != 0)
    {
        beep_ms(3, 200, 100);
        while (1);
    }

    accX    = (i2cData[0] << 8) | i2cData[1];
    accY    = (i2cData[2] << 8) | i2cData[3];
    accZ    = (i2cData[4] << 8) | i2cData[5];

    compAngleY = atan2(
            - accX
            , sqrt((float) accY * accY + (float) accZ * accZ)
            ) * RAD_TO_DEG;
    IMU_timer = micros();
}

void IMU_fillter(void)
{
    float dt = 0;

    if (i2cRead(IMU_ADDR, 0x3B, i2cData, 14) != 0)
    {
        flag |= STOP;
        return;
    }

    accX     = ((i2cData[0] << 8) | i2cData[1]);
    accY     = ((i2cData[2] << 8) | i2cData[3]);
    accZ     = ((i2cData[4] << 8) | i2cData[5]);
    //tempRaw  = (i2cData[6] << 8) | i2cData[7];
    //gyroXraw = (i2cData[8] << 8) | i2cData[9];
    gyroYraw = (i2cData[10] << 8) | i2cData[11];
    gyroZraw = (i2cData[12] << 8) | i2cData[13];

    now = micros();
    dt = (float) (now - IMU_timer) / 1000000;
    IMU_timer = now;

    pitch = atan2(
            - accX
            , sqrt((float) accY * accY + (float) accZ * accZ)
            ) * RAD_TO_DEG;
    gyroY = (float) gyroYraw / GYRO_FACTOR;
    gyroZ = (float) gyroZraw / GYRO_FACTOR;

    compAngleY = 0.93 * (compAngleY + gyroY * dt) + 0.07 * pitch;

#ifdef IMU_OUTPUT
    Serial.println(compAngleY);
#endif
}
