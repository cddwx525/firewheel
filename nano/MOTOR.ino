//
// Note: fast decay mode, 惯性太大。
//     ---------------------------------------------------
//     Direction               IN1    IN2      Decay mode
//     ---------------------   ----   ----     -----------
//     Forward(speed > 0)      1      PWM      brake(slow)
//     Backward(speed < 0)     PWM    1        brake(slow)
//     ---------------------------------------------------
// @param speed Relative to max PWM value.
//
void Motor(uint8_t id, float speed)
{
    int16_t pwm;

    pwm = PWM_MAX * speed;
    pwm = constrain(pwm
            , - PWM_MAX + config.motor_deadzone
            , PWM_MAX - config.motor_deadzone
            );

    if (pwm > 0)
            pwm += config.motor_deadzone;
    else if (pwm < 0)
            pwm -= config.motor_deadzone;

    switch (id)
    {
#define MOTOR_LIST_F(id, in1, in2, dir_bit)   \
    case id:                                  \
        if (pwm > 0)                          \
        {                                     \
            flag |= dir_bit;                  \
            digitalWrite(in1, HIGH);          \
            analogWrite(in2, 255 - pwm);      \
        }                                     \
        else                                  \
        {                                     \
            flag &= ~dir_bit;                 \
            digitalWrite(in2, HIGH);          \
            analogWrite(in1, 255 - (- pwm));  \
        }                                     \
        break;

    MOTOR_LIST

#undef MOTOR_LIST_F

    default:
        break;
    }
}

void Motor_stop(void)
{
#define MOTOR_LIST_F(id, in1, in2, dir_bit) Motor(id, 0);
    MOTOR_LIST
#undef MOTOR_LIST_F

    FACE_LED(0);
}


int Motor_measure(void)
{
    int count_L_now = 0;
    int count_R_now = 0;
    int count_L_delta = 0;
    int count_R_delta = 0;
    //static int count_L_delta_prev = 0;
    //static int count_R_delta_prev = 0;
    int count_avg = 0;

    count_L_now = count_L;
    count_R_now = count_R;

    count_L_delta = count_L_now - count_L_prev;
    count_R_delta = count_R_now - count_R_prev;
    //count_L_delta = 0.3 * (count_L - count_L_prev) + 0.7 * count_L_delta_prev;
    //count_R_delta = 0.3 * (count_R - count_R_prev) + 0.7 * count_L_delta_prev;
    //count_L_delta_prev = count_L_delta;
    //count_L_delta_prev = count_R_delta;

    count_L_prev = count_L_now;
    count_R_prev = count_R_now;

    count_avg = (count_L_delta + count_R_delta) / 2;

    if (log_flag & LOG_MOTOR_SPEED)
    {
        Serial.print(F("count_L:"));
        Serial.print(count_L);
        Serial.print(F(" count_L_delta:"));
        Serial.print(count_L_delta);
        Serial.print(F(" count_R:"));
        Serial.print(count_R);
        Serial.print(F(" count_R_delta:"));
        Serial.print(count_R_delta);
        Serial.print(F("\n"));
    }

    return count_avg;
}

void Motor_control(void)
{
    //int d = 0;
    float speed = 0;

    beep_ms(1, 100, 0);
    Serial.print(F("[info]: Motor L -> R.\n"));
    reset_state();
    while (! button_pressed())
    {
        flag |= DIR_L;
        speed = (count_L - count_R) * 0.1;
        Motor(RHT, speed);
    }
    beep_ms(2, 100, 100);
    Motor_stop();
    while (button_pressed());
    delay(DEBOUNCE_TIME);

    //beep_ms(1, 100, 0);
    //Serial.print(F("[info]: Motor L -> R force x7.\n"));
    //reset_state();
    //while(! button_pressed())
    //{
    //    flag |= DIR_L;
    //    Motor(RHT, (count_L - count_R) * 20);
    //    Motor(LFT, (count_R - count_L) * 3);
    //}
    //beep_ms(2, 100, 100);
    //Motor_stop();
    //while (button_pressed());
    //delay(DEBOUNCE_TIME);

    //beep_ms(1, 100, 0);
    //Serial.print(F("[info]: Motor L <-> R.\n"));
    //reset_state();
    //while(! button_pressed())
    //{
    //    //
    //    // L <-> R not working
    //    //
    //    //flag |= DIR_L;
    //    //flag |= DIR_R;
    //    //Motor(RHT, (count_L - count_R) * 20);
    //    //Motor(LFT, (count_R - count_L) * 20);
    //}
    //beep_ms(2, 100, 100);
    //Motor_stop();
    //while (button_pressed());
    //delay(DEBOUNCE_TIME);

    //beep_ms(1, 100, 0);
    //Serial.print(F("[info]: Motor L -> R mono dir.\n"));
    //reset_state();
    //while(! button_pressed())
    //{
    //    //
    //    // L -> R 棘轮
    //    //
    //    flag |= DIR_L;
    //    if (count_L - count_L_prev > 0)
    //            d += count_L - count_L_prev;
    //    count_L_prev = count_L;
    //    Motor(RHT, (d - count_R) * 10);
    //}
    //beep_ms(2, 100, 100);
    //Motor_stop();
    //while (button_pressed());
    //delay(DEBOUNCE_TIME);

    //beep_ms(1, 100, 0);
    //Serial.print(F("[info]: Motor L reflect.\n"));
    //reset_state();
    //while(! button_pressed())
    //{
    //    //
    //    // L 回弹 Not working
    //    //
    //    Motor(LFT, count_L * (- 10));
    //    delay(10);
    //}
    //beep_ms(2, 100, 100);
    //Motor_stop();
    //while (button_pressed());
    //delay(DEBOUNCE_TIME);

    while (! button_pressed());
    while (button_pressed());
    delay(DEBOUNCE_TIME);

    beep_ms(1, 100, 0);
    Serial.print(F("[info]: Motor L sin.\n"));
    reset_state();
    while(! button_pressed())
    {
        flag |= DIR_L;
        speed = sin((float) count_L / 10) * 0.20;
        if (abs(speed) < 0.1)
                speed = 0;
        Motor(LFT, speed);
    }
    beep_ms(2, 100, 100);
    Motor_stop();
    while (button_pressed());
    delay(DEBOUNCE_TIME);

    //beep_ms(1, 100, 0);
    //Serial.print(F("[info]: Motor L limit angle.\n"));
    //reset_state();
    //while(! button_pressed())
    //{
    //    //
    //    // L 限位 not working
    //    //
    //    if (count_L < (- 100))
    //            speed = (- 100 - count_L) * 5;
    //    else
    //            speed = 0;
    //    Motor(LFT, speed);
    //}
    //beep_ms(2, 100, 100);
    //Motor_stop();
    //while (button_pressed());
    //delay(DEBOUNCE_TIME);

    while (! button_pressed());
    while (button_pressed());
    delay(DEBOUNCE_TIME);

    beep_ms(1, 100, 0);
    Serial.print(F("[info]: Motor L low gravity.\n"));
    reset_state();
    while(! button_pressed())
    {
        //
        // NOTE: delay(90), OK, delay(100) may cause speedup.
        //
        speed = count_L - count_L_prev;
        count_L_prev = count_L;
        Motor(LFT, speed * 0.7 / PWM_MAX);
        delay(90);
    }
    beep_ms(2, 100, 100);
    Motor_stop();
    while (button_pressed());
    delay(DEBOUNCE_TIME);
}

