uint8_t button_pressed(void)
{
    return KEY_PRESSED(PIN_BUTTON_INR, PIN_BUTTON_BIT, PIN_BUTTON_PRESS_LEVEL);
}

uint8_t get_key_event(uint8_t (* pressed)(void))
{
    uint32_t start = 0;

    delay(DEBOUNCE_TIME);
    start = millis();
    while (1)
    {
        if (! pressed())
        {
            delay(DEBOUNCE_TIME);
            return KEY_EVENT_SHORT;
        }

        if (millis() - start >= KEY_LONG_T)
                return KEY_EVENT_LONG;
    }
}

