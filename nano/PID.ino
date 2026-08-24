void angle_PID_compute(void)
{
    float output_L, output_R;

    now = micros();
    if (now - angle_PID_timer < ANGLE_PID_T)
            return;

    angle_PID_timer = now;

    angle_error = angle_setpoint - compAngleY;
    angle_integral += angle_error;
    angle_output = - (config.P_angle * angle_error
            + config.I_angle * angle_integral
            + config.D_angle * (- gyroY));

    if (flag & TURNING)
    {
        turn_integral = 0;
    }
    else
    {
        turn_integral += gyroZ;
        turn_output = config.P_turn * gyroZ + config.I_turn * turn_integral;
    }

    output_L = angle_output + turn_output;
    output_R = angle_output - turn_output;

#ifdef MOTOR_ENABLE
    Motor(LFT, output_L);
    Motor(RHT, output_R);
#endif
}

void speed_PID_compute(void)
{
    float speed = 0;
    static uint8_t count_overspeed = 0;

    now = micros();
    if (now - speed_PID_timer < SPEED_PID_T)
            return;

    speed_PID_timer = now;

    speed = (float) Motor_measure() / MAX_SPEED;

    //Serial.print(speed);
    //Serial.print(F("\n"));
    if (fabs(speed) > 0.5)
    {
        count_overspeed ++;
        if (count_overspeed >= (500000 / SPEED_PID_T))
                flag |= STOP;
    }
    else
    {
        count_overspeed = 0;
    }

    speed_error = speed_setpoint - speed;
    speed_integral += speed_error;
    angle_setpoint = config.P_speed * speed_error
            + config.I_speed * speed_integral;

    angle_setpoint = constrain(
            angle_setpoint
            , - FALLDOWN_ANGLE
            , FALLDOWN_ANGLE
            );
}
