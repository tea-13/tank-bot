#define MAX_SPEED 170   // максимальная скорость моторов (0-255)

#define MOTOR_TEST 0    // тест моторов
// при запуске крутятся ВПЕРЁД по очереди:
// BL - задний левый
// BR - задний правый

// пины драйверов (_B должен быть ШИМ)
#define MOTOR1_A 2
#define MOTOR1_B 3  // ШИМ!
#define MOTOR2_A 4
#define MOTOR2_B 5  // ШИМ!

// пины сервоприводов
#define SERVO_R 6
#define SERVO_L 7

#include <GyverMotor.h>
#include <GyverOLED.h>
#include <Servo.h>

// создание экземпляров для серв
Servo servoR;
Servo servoL;

// тут можно поменять моторы местами
GMotor motorBR(DRIVER2WIRE, MOTOR1_A, MOTOR1_B, HIGH);
GMotor motorBL(DRIVER2WIRE, MOTOR2_A, MOTOR2_B, HIGH);

// создание экземпляря oled дисплея
GyverOLED<SSH1106_128x64> oled;

char inChar;
String input = "                      ";
byte currCharN = 0;


void setup() 
{
  Serial.begin(115200);
  Serial.println("start");

  servoTuning();

  oled.init();  // инициализация
  oledTuning();

  PWMOverclock();  // Разгон шима, чтоб не пищал 
  motorsTuning();  // Настройка моторов

#if (MOTOR_TEST == 1)
  motorsTest1();   // тест моторов
#endif

#if (MOTOR_TEST == 2)
  motorsTest2();   // тест моторов
#endif  
} 


void loop() 
{
  if (Serial.available() > 0)
  {
    inChar = Serial.read();
    if (inChar == '\n')
    {
      return;
    }
    input[currCharN++] = inChar;
    
    if (inChar == '\r')
    {
      if (input[0] == 'm' && input[1] == '2')
      {
        updateMotors2();
        delayMicroseconds(100);

      }
      else if (input[0] == 's')
      {
        switch(input[1])
        {
          case 'r':
            servoRotation(1);
            Serial.println("Right servo");
            break;
          case 'l':
            servoRotation(2);
            Serial.println("Left servo");
            break;
          default:
            motorsStop();
            Serial.println("Stop motors");
            break;
        }
      }
      else if (input[0] == 'e')
      {
        switch(input[2])
        {
          case 'b':
            blink();
            Serial.println("Eye blink");
            break;
          case 'l':
            break;
          default:
            Serial.println("No data");
            break;
        }
      }

      inputClear();
      currCharN = 0;
    }
    if (currCharN > 31)
    {
      inputClear();
      currCharN = 0;
    }
  }
}

// Serial funk
void inputClear()
{
  for (int i = 0; i < 32; i++)
  {
    input[i] = ' ';
  }
}


// Servo funk
void servoTuning()
{
  servoR.attach(SERVO_R);
  servoL.attach(SERVO_L);
}


void servoRotation(byte servoN)
{
  String buf_1 = input.substring(3, 32);
  int ang = buf_1.toInt();    // преобразуем во int
  Serial.println("Angle " + String(ang));

  if (servoN == 1)
    servoR.write(ang);
  else if (servoN == 2)
    servoL.write(ang);
  else
    Serial.println("Wrong servo number");
}


// Oled funk
void oledTuning()
{
  oled.clear();
  oled.roundRect(16, 16, 48, 48, OLED_FILL);  // аналогично скруглённый прямоугольник
  oled.roundRect(80, 16, 112, 48, OLED_FILL);  // аналогично скруглённый прямоугольник
  oled.update();
}

void blink()
{
  
}


// Motors funk
void PWMOverclock()
{
  // чуть подразгоним ШИМ https://alexgyver.ru/lessons/pwm-overclock/
  // Пины D3 и D11 - 980 Гц
  TCCR2B = 0b00000100;  // x64
  TCCR2A = 0b00000011;  // fast pwm

  // Пины D9 и D10 - 976 Гц
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00001011;  // x64 fast pwm
}


void motorsTest1()
{
  Serial.println("Test 1");
  Serial.println("back left");
  motorBL.run(FORWARD, 100);
  delay(3000);
  motorBL.run(STOP);
  delay(1000);
  Serial.println("back right");
  motorBR.run(FORWARD, 100);
  delay(3000);
  motorBR.run(STOP);
}


void motorsTest2()
{
  Serial.println("Test 2");
  
  Serial.println("back left");
  for (int i=100; i < 200; i += 10)
  {
    motorBL.run(FORWARD, i);
    Serial.println(i);
    delay(250);
  }
  motorBL.run(STOP);
  delay(1000);
  
  Serial.println("back right");
  for (int i=100; i < 200; i += 10)
  {
    motorBR.run(FORWARD, i);
    Serial.println(i);
    delay(250);
  }
  motorBR.run(STOP);
}


void motorsTuning()
{
  // направление глобального вращегия моторов
  motorBR.setDirection(REVERSE);
  motorBL.setDirection(FORWARD);

  // минимальный сигнал на мотор
  motorBR.setMinDuty(30);
  motorBL.setMinDuty(30);

  // режим мотора в АВТО
  motorBR.setMode(AUTO);
  motorBL.setMode(AUTO);

  // скорость плавности
  motorBR.setSmoothSpeed(60);
  motorBL.setSmoothSpeed(60);
}


void motorsStop()
{
  motorBR.setSpeed(0);
  motorBL.setSpeed(0);
}


void updateMotors2()
{
  String vals = input.substring(3, 32);
  byte dividerIndex = vals.indexOf(' ');   // ищем индекс разделителя
  String buf_1 = vals.substring(0, dividerIndex);    // создаём строку с первым числом
  String buf_2 = vals.substring(dividerIndex + 1);   // создаём строку со вторым числом
  int val_1 = buf_1.toInt();    // преобразуем во int
  int val_2 = buf_2.toInt();    // ...

  Serial.println("Parse data2: " + String(val_1) + " " + String(val_2));
  changeMotorsSpeed(val_1, val_2);
}


void changeMotorsSpeed(int speedBR, int speedBL)
{
  motorBR.setSpeed(speedBR);
  motorBL.setSpeed(speedBL);
}