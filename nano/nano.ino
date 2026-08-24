/**********************************************************************
说明:这是我做的一个小项目-自平衡机器人Nano的固件程序,机器人的实际演示
可以看这里http://pengzhihui.xyz/2015/12/09/nano/
项目使用方法:把Adafruit_GFX和Adafruit_SSD1306文件夹放入IDE的libraries
文件夹，打开Nano.ino就可以编译成功了，推荐使用最新版本IDE.引脚定义在程
序中用宏标出，需要修改PID参数以适应不同的机械硬件，更多疑问可以联系邮
箱 593245898@qq.com                            pz_cloud     2015.6.25
**********************************************************************/

#include <Wire.h>
#include <EEPROM.h>

////////////////////////////////////////////////////////////////////////////////
//
// 调试选项
//
////////////////////////////////////////////////////////////////////////////////
//#define TIMING_DEBUG
//#define IMU_OUTPUT
//#define SONIC_OUTPUT
#define SPEED_LOOP_ENABLE
#define MOTOR_ENABLE
#define SONIC_ENABLE

#ifdef TIMING_DEBUG
// TIMING consume 596 us (2026-08-18)
#define TIMING()        do \
        { \
            Serial.print(micros()); \
            Serial.print(F(" ")); \
        } while(0)
#define TIMING_HEAD()   do \
        { \
            Serial.println(); \
            TIMING(); \
        }while(0)
#else
#define TIMING()
#define TIMING_HEAD()
#endif


////////////////////////////////////////////////////////////////////////////////
//
// 标志位
//
////////////////////////////////////////////////////////////////////////////////
//
// Bit define for flag.
//
//     7 6 5 4 3 2 1 0
//     _ _ _ _ _ _ _ _
//     x | | | | | | |
//       | | | | | | +-- RUNNING
//       | | | | | +---- TURNING
//       | | | | +------ STOP
//       | | | +-------- DIR_L
//       | | +---------- DIR_R
//       | +------------ COMDONE
//       +-------------- CTRLING
//
#define RUNNING     0x01
#define TURNING     0x02
#define STOP        0x04
#define DIR_L       0x08
#define DIR_R       0x10
#define COMDONE     0x20
#define CTRLING     0x40

//
// Bit define for log_flag.
//
//    7 6 5 4 3 2 1 0
//    _ _ _ _ _ _ _ _
//    x x x x | | | |
//            | | | +-- LOG_VALUE
//            | | +---- LOG_MOTOR_SPEED
//            | +------ LOG_ANGLE_PID
//            +-------- LOG_SPEED_PID
//
#define LOG_VALUE       0x01
#define LOG_MOTOR_SPEED 0x02
#define LOG_ANGLE_PID   0x04
#define LOG_SPEED_PID   0x08

////////////////////////////////////////////////////////////////////////////////
//
// General
//
////////////////////////////////////////////////////////////////////////////////
#define CONFIG_MARK     20260810
#define PRINT_PRECISE   8

#define DEBUG_T         50  // ms
#define DEBOUNCE_TIME   20  // ms
#define KEY_SHORT_T     200 // ms
#define KEY_LONG_T      2000 // ms
#define KEY_EVENT_SHORT 1
#define KEY_EVENT_LONG  2
#define KEY_PRESSED(pin_reg, pin_bit, level) \
        ( \
            (level) ? \
                bit_is_set(pin_reg, pin_bit) \
                : \
                bit_is_clear(pin_reg, pin_bit) \
        )

////////////////////////////////////////////////////////////////////////////////
//
// 引脚定义
//
////////////////////////////////////////////////////////////////////////////////
//
// Motor
//
#define LFT             0
#define RHT             1
#define PIN_MOTOR_L_ENC_D       2  // PD2
#define PIN_MOTOR_L_IN1_D       5  // PD5
#define PIN_MOTOR_L_IN1_M       OUTPUT
#define PIN_MOTOR_L_IN2_D       9  // PB1
#define PIN_MOTOR_L_IN2_M       OUTPUT

#define PIN_MOTOR_R_ENC_D       3  // PD3
#define PIN_MOTOR_R_IN1_D       6  // PD6
#define PIN_MOTOR_R_IN1_M       OUTPUT
#define PIN_MOTOR_R_IN2_D       10 // PB2
#define PIN_MOTOR_R_IN2_M       OUTPUT

#define MOTOR_LIST \
        MOTOR_LIST_F(LFT, PIN_MOTOR_L_IN1_D, PIN_MOTOR_L_IN2_D, DIR_L) \
        MOTOR_LIST_F(RHT, PIN_MOTOR_R_IN1_D, PIN_MOTOR_R_IN2_D, DIR_R)

#define ENC_TUNE_STEP           100     // 100 pulses as a change
#define MAX_SPEED               185     // 185/50 ms
#define SPEED_PID_T             50000   // 50 ms
#define SPEED_INTEGRAL_LIMIT    10000   //
#define ANGLE_PID_T             10000   // 10 ms
#define ANGLE_INTEGRAL_LIMIT    2000    // (2 s / 10 ms) * 20 deg
#define MOTOR_MEASURE_T         50000   // 50 ms
#define PWM_MAX                 255

//
// Sonic
//
#define PIN_TRIG_D      8 // PB0
#define PIN_TRIG_OUTR   PORTB
#define PIN_TRIG_BIT    0
#define PIN_TRIG_M      OUTPUT
#define PIN_ECHO_D      7 // PD7
#define PIN_ECHO_INR    PIND
#define PIN_ECHO_BIT    7
#define PIN_ECHO_M      INPUT
#define SONIC_FACTOR    0.017   // 0.0343 cm/us / 2(343 m/s / 2)
#define SONIC_TIME      1500    // 1500 us
#define SONIC_T         100000  // 100 ms

//
// IMU
//
#define IMU_ADDR        0x68
#define GYRO_FACTOR     131     // -250 ~ +250
#define FALLDOWN_ANGLE  30


// Buzzer
#define PIN_BUZZER_D        4
#define PIN_BUZZER_M        OUTPUT
#define BUZZER_ON()         digitalWrite(PIN_BUZZER_D, LOW)
#define BUZZER_OFF()        digitalWrite(PIN_BUZZER_D, HIGH)

// LED
#define PIN_LED_D       11
#define PIN_LED_M       OUTPUT
#define FACE_LED(x)     analogWrite(PIN_LED_D, 255 - x)  //LED 亮度函数

// Button
#define PIN_BUTTON_D    14
#define PIN_BUTTON_INR  PINC
#define PIN_BUTTON_BIT  0
#define PIN_BUTTON_M    INPUT_PULLUP
#define PIN_BUTTON_PRESS_LEVEL    LOW

////////////////////////////////////////////////////////////////////////////////
//
// 函数声明
//
////////////////////////////////////////////////////////////////////////////////
void Config_load(void);
void Config_save(void);
uint8_t i2cWrite(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t size
        , bool sendStop);
uint8_t i2cRead(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t size);
void IMU_init(void);
void IMU_fillter(void);
uint8_t button_pressed(void);
uint8_t get_key_event(uint8_t (* pressed)(void));
void Motor(uint8_t id, float speed);
void Motor_stop(void);
void Motor_control(void);
int Motor_measure(void);
void angle_PID_compute(void);
void speed_PID_compute(void);
void Sonic(void);
void get_uart(void);
void set_value(void);
void print_info(void);
void reset_state(void);
void beep_ms(uint8_t n, uint16_t on, uint16_t off);
void adjust_balance_angle(void);
void Run(void);

////////////////////////////////////////////////////////////////////////////////
//
// 变量声明
//
////////////////////////////////////////////////////////////////////////////////
//
// Gyroscope accelerometer
//
uint32_t IMU_timer = 0;
uint8_t i2cData[14];
int16_t accX, accY, accZ;   // Linear acceleration(g)
int16_t gyroYraw, gyroZraw = 0;
int16_t tempRaw = 0;
float gyroY, gyroZ = 0;     // Rotational speed(°/s)
float compAngleY = 0;
float pitch = 0;

//
// Motor
//
volatile int count_L = 0;
volatile int count_R = 0;
int count_L_prev = 0;
int count_R_prev = 0;
uint32_t motor_measure_timer = 0;

//
// Sonic
//
uint32_t SONIC_timer = 0;
float distance_cm, joy_x, joy_y;

//
// 串口缓存数据
//
char comdata[19];
uint8_t data_p;

//
// PID
//
uint32_t angle_PID_timer = 0;
float angle_setpoint = 0;
float angle_error = 0;
float angle_integral = 0;
float angle_output = 0;
//float P_angle = 0;
//float I_angle = 0;
//float D_angle = 0;

uint32_t speed_PID_timer = 0;
float speed_setpoint = 0;
float speed_error = 0;
float speed_integral = 0;
float speed_output = 0;
//float P_speed = 0;
//float I_speed = 0;

float turn_integral = 0;
float turn_output = 0;
//float P_turn = 0;
//float I_turn = 0;

// Config
struct config {
        uint32_t mark;
        uint8_t motor_deadzone;
        float balance_angle;
        float P_angle;
        float I_angle;
        float D_angle;
        float P_speed;
        float I_speed;
        float P_turn;
        float I_turn;
        } config;

// debug
uint32_t debug_timer = 0;
uint32_t now = 0;

//
// Flag
//
uint8_t flag = 0;
uint8_t log_flag = 0;


////////////////////////////////////////////////////////////////////////////////
//
// Setup
//
////////////////////////////////////////////////////////////////////////////////
void setup()
{
    ////////////////////////////////////////////////////////////////////////////
    // Timer
    ////////////////////////////////////////////////////////////////////////////
    //
    // Timer0
    //    PWM: D5(PD5), D6(PD6)
    //    fast mode, 976 Hz (default)
    //

    //
    // Timer1
    //    PWM: D9(PB1), D10(PB2)
    //    fast mode, 976 Hz
    //    phase correct mode, 490 Hz (default)
    //
    DDRB |= _BV(DDB1) | _BV(DDB2);
    TCCR1A = 0;
    TCCR1B = 0;
    // Fast PWM 8 bit (Mode 5)
    TCCR1A |= _BV(WGM10);
    TCCR1B |= _BV(WGM12);
    // Non-inverting output
    TCCR1A |= _BV(COM1A1) | _BV(COM1B1);
    // Prescaler = 64, 976 Hz
    TCCR1B |= _BV(CS11) | _BV(CS10);

    ////////////////////////////////////////////////////////////////////////////
    // PIN
    ////////////////////////////////////////////////////////////////////////////
    // Motor
    pinMode(PIN_MOTOR_L_IN1_D, PIN_MOTOR_L_IN1_M);
    pinMode(PIN_MOTOR_L_IN2_D, PIN_MOTOR_L_IN2_M);
    pinMode(PIN_MOTOR_R_IN1_D, PIN_MOTOR_R_IN1_M);
    pinMode(PIN_MOTOR_R_IN2_D, PIN_MOTOR_R_IN2_M);
    Motor(LFT, 0);
    Motor(RHT, 0);

    // Sonic
    pinMode(PIN_TRIG_D, PIN_TRIG_M);
    pinMode(PIN_ECHO_D, PIN_ECHO_M);
    digitalWrite(PIN_TRIG_D, LOW);

    // Buzzer
    pinMode(PIN_BUZZER_D, PIN_BUZZER_M);

    // LED
    pinMode(PIN_LED_D, PIN_LED_M);
    FACE_LED(0);

    // Button
    pinMode(PIN_BUTTON_D, PIN_BUTTON_M);

    // 编码器中断
    attachInterrupt(digitalPinToInterrupt(PIN_MOTOR_L_ENC_D)
            , update_left_encoder, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_MOTOR_R_ENC_D)
            , update_right_encoder, FALLING);

    ////////////////////////////////////////////////////////////////////////////
    // Init
    ////////////////////////////////////////////////////////////////////////////
    // EEPROM
    Config_load();

    // Communication
    Serial.begin(115200);   // 蓝牙串口
    Wire.begin();           // I2C总线

    // MPU6050
    IMU_init();

    TIMING();
    TIMING();
}

////////////////////////////////////////////////////////////////////////////////
//
// Loop
//
////////////////////////////////////////////////////////////////////////////////
void loop()
{
    //
    // Standby mode.
    //
    beep_ms(1, 100, 0);
    Serial.print(F("[info]: Standby mode.\n"));
    while (1)
    {
        IMU_fillter();

        get_uart();
        set_value();

        now = micros();
        if (now - motor_measure_timer > MOTOR_MEASURE_T)
        {
            motor_measure_timer = now;
            Motor_measure();
        }

        if (button_pressed())
        {
            if (get_key_event(button_pressed) == KEY_EVENT_LONG)
                    break;

            beep_ms(1, 50, 0);

            delay(2000);
            Run();
            continue;
        }

        if (flag & RUNNING)
                Run();

        //Serial.print(compAngleY); 
        //Serial.print(F(" "));
        //Serial.print(gyroY);
        //Serial.print(F("\n"));
        if (fabs(config.balance_angle - compAngleY) < 0.2 && fabs(gyroY) < 0.5)
                Run();
    }
    beep_ms(2, 100, 100);
    while (button_pressed());
    delay(DEBOUNCE_TIME);

    while (! button_pressed());
    while (button_pressed());
    delay(DEBOUNCE_TIME);

    //
    // Setting mode.
    //
    adjust_balance_angle();

    while (! button_pressed());
    while (button_pressed());
    delay(DEBOUNCE_TIME);

    //
    // Advance mode.
    //
    Motor_control();

    while (! button_pressed());
    while (button_pressed());
    delay(DEBOUNCE_TIME);

    beep_ms(4, 100, 50);
    delay(500);
}

////////////////////////////////////////////////////////////////////////////////
//
// 中断函数
//
////////////////////////////////////////////////////////////////////////////////
void update_left_encoder()
{
    if (flag & DIR_L)
            count_L ++;
    else
            count_L --;
}

void update_right_encoder()
{
    if (flag & DIR_R)
            count_R ++;
    else
            count_R --;
}
