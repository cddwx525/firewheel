void run_sonic(void)
{
    bitClear(PIN_TRIG_OUTR, PIN_TRIG_BIT);
    bitSet(PIN_TRIG_OUTR, PIN_TRIG_BIT);
    delayMicroseconds(10);
    bitClear(PIN_TRIG_OUTR, PIN_TRIG_BIT);

    distance_cm = pulseIn(PIN_ECHO_D, HIGH, SONIC_TIME) * SONIC_FACTOR;

#ifdef SONIC_OUTPUT
    Serial.println(distance_cm);
#endif

    if (distance_cm > 10 && distance_cm < 15 )
    {
        speed_setpoint = 0.3;
        FACE_LED(255);
    }
    else if (distance_cm < 8 && distance_cm > 0)
    {
        speed_setpoint = - 0.3;
        FACE_LED(255);
    }
    else
    {
        speed_setpoint = 0;
        FACE_LED(0);
    }
}
