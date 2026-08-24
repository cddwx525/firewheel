void get_uart(void)
{
    char c;

    while (Serial.available() > 0)
    {
        c = char(Serial.read());
        if (c == ';')
        {
            comdata[data_p] = c;
            data_p = 0;
            flag |= COMDONE;
        }
        else if (data_p < sizeof(comdata) - 1)
        {
            comdata[data_p ++] = c;
        }
    }
}

void set_value(void)
{
    uint8_t i;
    uint8_t motor_id;
    int16_t motor_pwm;

    if (! (flag & COMDONE))
            return;

    switch (comdata[0])
    {
    case 'R':
        //
        // Run.
        //
        switch (comdata[1])
        {
        case 'R':
            flag |= RUNNING;

            Serial.print(F("[info]: Start ...\n"));
            break;

        case 'S':
            flag |= STOP;

            Serial.print(F("[info]: Stop ...\n"));
            break;
        }
        break;

    case 'T':
        //
        // Tuning
        //
        switch (comdata[1])
        {
        case 'M':
            strtok(comdata, ",");
            motor_id = atoi(strtok(NULL, ","));
            motor_pwm = atoi(strtok(NULL, ";"));

            Motor(motor_id, motor_pwm);

            Serial.print(F("\n"));
            Serial.print(F("----------------------------------------\n"));
            Serial.print(F("motor_id : "));
            Serial.print(motor_id);
            Serial.print(F("\n"));
            Serial.print(F("motor_pwm: "));
            Serial.print(motor_pwm);
            Serial.print(F("\n"));
            Serial.print(F("----------------------------------------\n"));
            break;

        case 'D':
            strtok(comdata, ",");
            config.motor_deadzone = atoi(strtok(NULL, ";"));

            Serial.print(F("\n"));
            Serial.print(F("----------------------------------------\n"));
            Serial.print(F("motor_deadzone: "));
            Serial.print(config.motor_deadzone);
            Serial.print(F("\n"));
            Serial.print(F("----------------------------------------\n"));
            break;

        case 'B':
            strtok(comdata, ",");
            config.balance_angle = atof(strtok(NULL, ";"));
            angle_setpoint = config.balance_angle;

            Serial.print(F("\n"));
            Serial.print(F("----------------------------------------\n"));
            Serial.print(F("balance_angle : "));
            Serial.print(config.balance_angle);
            Serial.print(F("\n"));
            Serial.print(F("----------------------------------------\n"));
            break;

        case 'A':
            strtok(comdata, ",");
            config.P_angle = atof(strtok(NULL, ","));
            config.I_angle = atof(strtok(NULL, ","));
            config.D_angle = atof(strtok(NULL, ";"));
            angle_integral = 0;

            print_info();
            break;

        case 'S':
            strtok(comdata, ",");
            config.P_speed = atof(strtok(NULL, ","));
            config.I_speed = atof(strtok(NULL, ";"));
            speed_integral = 0;
            count_L = 0;
            count_R = 0;

            print_info();
            break;

        case 'T':
            strtok(comdata, ",");
            config.P_turn = atof(strtok(NULL, ","));
            config.I_turn = atof(strtok(NULL, ";"));
            turn_integral = 0;

            print_info();
            break;

        case 'W':
            strtok(comdata, ";");

            Config_save();
            Serial.print(F("[info]: Saved.\n"));
            break;

        case 'C':
            strtok(comdata, ";");

            Config_load();
            Serial.print(F("[info]: Loaded.\n"));
            break;
        }
        break;

    case 'L':
        //
        // Log
        //
        switch (comdata[1])
        {
        case 'M':
            strtok(comdata, ",");
            i = atoi(strtok(NULL, ";"));

            if (i == 0)
                    log_flag &= ~LOG_MOTOR_SPEED;
            else
                    log_flag |= LOG_MOTOR_SPEED;
            break;

        case 'A':
            strtok(comdata, ",");
            i = atoi(strtok(NULL, ";"));

            if (i == 0)
                    log_flag &= ~LOG_ANGLE_PID;
            else
                    log_flag |= LOG_ANGLE_PID;
            break;

        case 'S':
            strtok(comdata, ",");
            i = atoi(strtok(NULL, ";"));

            if (i == 0)
                    log_flag &= ~LOG_SPEED_PID;
            else
                    log_flag |= LOG_SPEED_PID;
            break;

        case 'V':
            strtok(comdata, ",");
            i = atoi(strtok(NULL, ";"));

            if (i == 0)
                    log_flag &= ~LOG_VALUE;
            else
                    log_flag |= LOG_VALUE;
            break;
        }
        break;

    case 'G':
        //
        // Get
        //
        switch (comdata[1])
        {
        case 'C':
            print_info();
            break;
        case 'P':
            //
            // Not working for BTremote 0.6.3
            //
            Serial.print(F("P,"));
            Serial.print(config.P_angle);
            Serial.print(F(","));
            Serial.print(config.I_angle);
            Serial.print(F(","));
            Serial.print(config.D_angle);
            Serial.print(F(","));
            Serial.print(angle_setpoint);
            Serial.print("\n");
            break;

        case 'V':
            Serial.print(F("V,"));
            // Pitch simplify
            Serial.print(atan2(- accX, accZ) * RAD_TO_DEG);
            Serial.print(F(","));
            // GyroY 角速度
            Serial.print(gyroY);
            Serial.print(F(","));
            // 卡尔曼值
            Serial.print("0");
            Serial.print("\n");
            break;
        }
        break;

    case 'C':
        //
        // Joystick
        //
        // CJ,...
        //     0 < joy_x < 1
        //     0 < joy_y < 1
        // CM,...
        // CS,...
        //
        switch (comdata[1])
        {
        case 'J':
            strtok(comdata, ",");
            joy_x = atof(strtok(NULL, ","));
            joy_y = atof(strtok(NULL, ";"));

            flag |= CTRLING;
            speed_setpoint = joy_y;
            if (joy_x > 0.2 || joy_x < - 0.2)
            {
                flag |= TURNING;
                if (speed_setpoint > 0)
                        turn_output = joy_x / 4;
                else
                        turn_output = - joy_x / 4;
            }
            else
            {
                flag &= ~TURNING;
                turn_output = 0;
            }
            break;

        case 'M':
            strtok(comdata, ",");
            joy_x = atof(strtok(NULL, ","));
            joy_y = atof(strtok(NULL, ";"));

            flag |= CTRLING;
            speed_setpoint = (joy_x + 20);
            speed_setpoint = constrain(speed_setpoint, - 30, 30);

            if (joy_y > 10 || joy_y < - 10)
            {
                flag |= TURNING;
                if (speed_setpoint > 0)
                        turn_output = joy_y * 0.3;
                else
                        turn_output = - joy_y * 0.3;
            }
            else
            {
                flag &= ~TURNING;
                turn_output = 0;
            }
            break;

        case 'S':
            speed_setpoint = 0;
            turn_output = 0;
            flag &= ~TURNING;
            flag &= ~CTRLING;
            break;
        }
        break;
    }

    flag &= ~COMDONE;
}

void print_info(void)
{
    Serial.print(F("\n"));
    Serial.print(F("----------------------------------------\n"));

    Serial.print(F("compAngleY    : "));
    Serial.print(compAngleY, PRINT_PRECISE);
    Serial.print(F("\n"));

    Serial.print(F("\n"));
    Serial.print(F("balance_angle : "));
    Serial.print(config.balance_angle);
    Serial.print(F("\n"));

    Serial.print(F("\n"));
    Serial.print(F("motor_deadzone: "));
    Serial.print(config.motor_deadzone);
    Serial.print(F("\n"));

    Serial.print(F("\n"));
    Serial.print(F("P_angle       : "));
    Serial.print(config.P_angle, PRINT_PRECISE);
    Serial.print(F("\n"));
    Serial.print(F("I_angle       : "));
    Serial.print(config.I_angle, PRINT_PRECISE);
    Serial.print(F("\n"));
    Serial.print(F("D_angle       : "));
    Serial.print(config.D_angle, PRINT_PRECISE);
    Serial.print(F("\n"));

    Serial.print(F("\n"));
    Serial.print(F("P_speed       : "));
    Serial.print(config.P_speed, PRINT_PRECISE);
    Serial.print(F("\n"));
    Serial.print(F("I_speed       : "));
    Serial.print(config.I_speed, PRINT_PRECISE);
    Serial.print(F("\n"));

    Serial.print(F("\n"));
    Serial.print(F("P_turn        : "));
    Serial.print(config.P_turn, PRINT_PRECISE);
    Serial.print(F("\n"));
    Serial.print(F("I_turn        : "));
    Serial.print(config.I_turn, PRINT_PRECISE);
    Serial.print(F("\n"));

    Serial.print(F("----------------------------------------\n"));
}

void reset_state(void)
{
    count_L = 0;
    count_R = 0;
    count_L_prev = 0;
    count_R_prev = 0;

    angle_setpoint = config.balance_angle;
    angle_integral = 0;

    speed_integral = 0;

    turn_integral = 0;
}

void beep_ms(uint8_t n, uint16_t on, uint16_t off)
{
    uint8_t i;

    for (i = 0; i < n; i ++)
    {
        BUZZER_ON();
        delay(on);
        BUZZER_OFF();
        if (i + 1 != n)
                delay(off);
    }
}


void adjust_balance_angle(void)
{
    float balance_angle_backup = 0;
    float balance_angle_prev = 0;

    beep_ms(1, 100, 0);
    Serial.print(F("[info]: Balance angle adjust.\n"));

    balance_angle_backup = config.balance_angle;
    balance_angle_prev = config.balance_angle;
    reset_state();
    flag |= DIR_L;
    flag |= DIR_R;

    while (! button_pressed())
    {
        if (count_L - count_L_prev > ENC_TUNE_STEP)
        {
            count_L_prev = count_L;
            beep_ms(1, 10, 0);
            config.balance_angle -= 0.1;
        }

        if (count_R - count_R_prev > ENC_TUNE_STEP)
        {
            count_R_prev = count_R;
            beep_ms(1, 10, 0);
            config.balance_angle += 0.1;
        }

        if (config.balance_angle != balance_angle_prev)
        {
            if (
                    (fabs(config.balance_angle - 0) < 0.01)
                    &&
                    (fabs(balance_angle_prev - 0) > 0.01)
            )
            {
                config.balance_angle = 0;
                beep_ms(1, 200, 0);
            }
            balance_angle_prev = config.balance_angle;
            Serial.print(F("[info]: config.balance_angle = "));
            Serial.print(config.balance_angle);
            Serial.print(F("\n"));
        }
        delay(100);
    }
    if (get_key_event(button_pressed) == KEY_EVENT_LONG)
    {
        beep_ms(1, 300, 0);
        Config_save();
        angle_setpoint = config.balance_angle;
        Serial.print(F("[info]: Balance angle saved.\n"));
        while (button_pressed());
        delay(200);
    }
    else
    {
        config.balance_angle = balance_angle_backup;
    }

    beep_ms(2, 100, 100);
}


void Run(void)
{
    beep_ms(2, 50, 50);
    reset_state();
    flag |= RUNNING;
    while (1)
    {
        TIMING_HEAD();

        ///////////////////////////////////////////////////////////////////////////
        // Key
        ///////////////////////////////////////////////////////////////////////////
        if (button_pressed())
                break;

        if (flag & STOP)
                break;

        TIMING();

        ///////////////////////////////////////////////////////////////////////////
        // PID_loop + sonic
        ///////////////////////////////////////////////////////////////////////////
#ifdef SPEED_LOOP_ENABLE
        speed_PID_compute();    // 0.5 ms
        TIMING();
#endif

        IMU_fillter();          // 3 ms

        if (fabs(compAngleY) > FALLDOWN_ANGLE)
                flag |= STOP;
        TIMING();

        angle_PID_compute();    // 0.6 ms
        TIMING();

#ifdef SONIC_ENABLE
        now = micros();
        if ((now - SONIC_timer >= SONIC_T) && (! (flag & CTRLING)))
        {
            SONIC_timer = now;
            run_sonic();        // 2 ms
        }
        TIMING();
#endif

        ///////////////////////////////////////////////////////////////////////////
        // Log + UART
        ///////////////////////////////////////////////////////////////////////////
        now = millis();
        if (now - debug_timer > DEBUG_T)
        {
            debug_timer = now;

            if (log_flag & LOG_VALUE)
            {
                Serial.print(F("compAngleY:"));
                Serial.print(compAngleY);
                Serial.print(F(" gyroY:"));
                Serial.print(gyroY);
                Serial.print(F(" ptich:"));
                Serial.print(pitch);
                Serial.print(F(" ptich_s:"));
                Serial.print(atan2(- accX, accZ) * RAD_TO_DEG);
                Serial.print(F("\n"));
            }

            if (log_flag & LOG_ANGLE_PID)
            {
                Serial.print(F("Output:"));
                Serial.print(angle_output);

                Serial.print(F(" P_part:"));
                Serial.print(config.P_angle * angle_error);
                Serial.print(F(" error:"));
                Serial.print(angle_error, PRINT_PRECISE);

                Serial.print(F(" I_part:"));
                Serial.print(config.I_angle * angle_integral);
                Serial.print(F(" integ:"));
                Serial.print(angle_integral, PRINT_PRECISE);

                Serial.print(F(" D_part:"));
                Serial.print(config.D_angle * gyroY);
                Serial.print(F(" gyroY:"));
                Serial.print(gyroY, PRINT_PRECISE);

                Serial.print(F("\n"));
            }

            if (log_flag & LOG_SPEED_PID)
            {
                Serial.print(F("Output:"));
                Serial.print(angle_setpoint, PRINT_PRECISE);

                Serial.print(F(" L:"));
                Serial.print(count_L);
                Serial.print(F(" R:"));
                Serial.print(count_R);

                Serial.print(F(" P_part:"));
                Serial.print(config.P_speed * speed_error, PRINT_PRECISE);
                Serial.print(F(" error:"));
                Serial.print(speed_error, PRINT_PRECISE);

                Serial.print(F(" I_part:"));
                Serial.print(config.I_speed * speed_integral, PRINT_PRECISE);
                Serial.print(F(" integ:"));
                Serial.print(speed_integral, PRINT_PRECISE);

                Serial.print(F("\n"));
            }
        }
        TIMING();

        get_uart();
        set_value();
        TIMING();

        ///////////////////////////////////////////////////////////////////////////
        // PID_loop
        ///////////////////////////////////////////////////////////////////////////
#ifdef SPEED_LOOP_ENABLE
        speed_PID_compute();
        TIMING();
#endif

        angle_PID_compute();
        TIMING();
    }
    beep_ms(3, 50, 50);
    Motor_stop();

    flag &= ~STOP;
    flag &= ~RUNNING;

    while(button_pressed());
    delay(DEBOUNCE_TIME);
}
