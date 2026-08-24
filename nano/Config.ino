void Config_load(void)
{
    EEPROM.get(0, config);
    if (config.mark != CONFIG_MARK)
    {
        config.mark = CONFIG_MARK;

        config.motor_deadzone = 0;
        config.balance_angle = 0;
        config.P_angle = 0;
        config.I_angle = 0;
        config.D_angle = 0;

        config.P_speed = 0;
        config.I_speed = 0;

        config.P_turn = 0;
        config.I_turn = 0;

        EEPROM.put(0, config);
    }
    angle_setpoint = config.balance_angle;
}

void Config_save(void)
{
    EEPROM.put(0, config);
}
