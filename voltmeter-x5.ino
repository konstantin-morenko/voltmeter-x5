

/* Экран */
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(OLED_RESET);

/* Чтение и сглаживание */
#define NUM_READINGS 10
#define READING_DELAY 2

/* Преобразование АЦП в напряжение */
#define BASE_VOLTAGE 5.0
#define ADC_RESOLUTION 1024

#define VOLT_THRES 0.1 // Чувствительность отклонения для строки состояния

/* Аварийный сигнал */
#define VOLT_LIMIT 1.0 // Нижний предел для аварийного сигнала
#define TONE_OUTPUT 3
#define TONE_FREQ 500
#define TONE_LENGHT 500

int adcs[6]; // Результаты АЦП
float tvolt; // Общее напряжение
float volts[5]; // Напряжения

void setup() {
  init_screen();
  init_adc();
}

void loop() {
  read_adcs();
  convert_adcs();
  refresh_screen();
  alarm_check();
  delay(700);
}

/* Проверить и выдать сигнал тревоги */
inline void alarm_check() {
  if (volts[0] < VOLT_LIMIT) {
    tone(TONE_OUTPUT, TONE_FREQ, TONE_LENGHT);
  }
}

/* Обновить экран */
inline void refresh_screen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print(" U="); display.print(volts[0]);

  int i_max, i_min;
  float v_max = 0, v_min = BASE_VOLTAGE;
  for (int i = 0; i < 5; i++) {
    if(volts[i] > v_max) {
      v_max = volts[i];
      i_max = i;
    }
    if(volts[i] < v_min) {
      v_min = volts[i];
      i_min = i;
    }
  }

  display.setCursor(0, 8);
  display.print(String("U" + String(i_min) + String("=") + String(volts[i_min])));

  display.setCursor(0, 16);
  display.print(String("U" + String(i_max) + String("=") + String(volts[i_max])));

  display.setCursor(0, 24);
  display.print(make_checkline());

  show_perc(vtoperc(tvolt));
  display.display();
  
}

/* Пересчет напряжения в проценты заряда */
inline int vtoperc(float v) {
  int x = 100*v;
  int perc = -0.0116 * x * x + 12.614 * x - 3313;
  return perc;
}

/* Отобразить заряд в процентах */
inline void show_perc(int perc) {
    
  display.setTextColor(WHITE);
  
  display.setTextSize(4);
  display.setCursor(56, 0);
  display.print(perc);
  
  display.setTextSize(1);
  display.println("%"); 
}

/* Создать строку состояния аккумуляторов по напряжению */
inline String make_checkline() {
  
  float sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += volts[i];
  }
  float average = sum / 5;
  
  String check_line = "";
  for (int i = 0; i < 5; i++) {
    if(volts[i] < average - VOLT_THRES)  {
      check_line += String("0");
    }
    else {
      check_line += String("1");
    }
  }
  return check_line;
}

/* Преобразовать вывод АЦП в напряжения */
inline void convert_adcs() {
  tvolt = adcs[0] * BASE_VOLTAGE / ADC_RESOLUTION;
  for(int i = 1; i < 6; i++) {
    volts[i-1] = adcs[i] * BASE_VOLTAGE / ADC_RESOLUTION;
  }
}

/* Прочитать значения АЦП */
inline void read_adcs() {
  
  int x = 0;

  x = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    delay(READING_DELAY);
    x += analogRead(A0);
  }
  adcs[0] = x / NUM_READINGS;

  x = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    delay(READING_DELAY);
    x += analogRead(A1);
  }
  adcs[1] = x / NUM_READINGS;

  x = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    delay(READING_DELAY);
    x += analogRead(A2);
  }
  adcs[2] = x / NUM_READINGS;

  x = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    delay(READING_DELAY);
    x += analogRead(A3);
  }
  adcs[3] = x / NUM_READINGS;

  x = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    delay(READING_DELAY);
    x += analogRead(A6);
  }
  adcs[4] = x / NUM_READINGS;

  x = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    delay(READING_DELAY);
    x += analogRead(A7);
  }
  adcs[5] = x / NUM_READINGS;
  
}

/* Включить экран */
inline void init_screen() {
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.display();
  display.clearDisplay();
}

/* Настроить АЦП */
inline void init_adc() {

}
