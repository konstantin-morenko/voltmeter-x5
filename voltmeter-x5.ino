
/* ======================================== */
/* КОНСТАНТЫ */

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
#define V_TOTAL_LIMIT 1.0 // Нижний предел сигнала суммарного напряжения
#define TONE_OUTPUT 3
#define TONE_LENGHT 500
#define LED_OUTPUT 13
#define TONE_FREQ_TOTAL 500
#define TONE_FREQ_SINGLE 1000

/* Входы */
#define VTOTALIN A0
#define V1IN A1
#define V2IN A2
#define V3IN A3
#define V4IN A6
#define V5IN A7

/* Сигнальные светодиоды */
#define VTOTALLED 5
#define VLEDSTART 6

/* Глобальные переменные */
int adcs[6]; // Результаты АЦП
float tvolt; // Общее напряжение
float volts[5]; // Напряжения


/* ======================================== */
/* ИНИЦИАЛИЗАЦИЯ */

void setup() {
  init_screen();
  init_adc();
  init_led();
}

void loop() {
  read_adcs();
  convert_adcs();
  refresh_screen();
  alarm_check();
  delay(700);
}

/* Включить экран */
inline void init_screen() {
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();
}

/* Настроить АЦП */
inline void init_adc() {

}

/* Настроить световую индикацию */
inline void init_led() {
}


/* ======================================== */
/* СИГНАЛИЗАЦИЯ */

/* Проверить сигнал тревоги */
inline bool alarm_check() {
  if(tvolt < V_TOTAL_LIMIT) {
    blink_led(VTOTALLED);
    raise_alarm(TONE_FREQ_TOTAL);
  }
  else {
    turn_off_led(VTOTALLED);
  }
}

void blink_led(int led) {
  digitalWrite(led, !digitalRead(led)); // Invert pin
}

void turn_off_led(int led) {
  digitalWrite(led, 0);
}

/* Выдать тональный сигнал тревоги */
inline void raise_alarm(int freq) {
  tone(TONE_OUTPUT, freq, TONE_LENGHT);
}


/* ======================================== */
/* ИНДИКАЦИЯ */

/* Обновить экран */
inline void refresh_screen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print(" U="); display.print(tvolt);

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

  const int i_shift = 1;
  display.setCursor(0, 8);
  display.print(String("U" + String(i_min + i_shift) + String("=") + String(volts[i_min])));

  display.setCursor(0, 16);
  display.print(String("U" + String(i_max + i_shift) + String("=") + String(volts[i_max])));

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
    int led_output = VLEDSTART + i;
    if(volts[i] < average - VOLT_THRES)  {
      check_line += String("0");
      blink_led(led_output);
      raise_alarm(TONE_FREQ_SINGLE);
    }
    else {
      check_line += String("1");
      turn_off_led(led_output);
    }
    check_line += String(" ");
  }
  return check_line;
}


/* ======================================== */
/* ЧТЕНИЕ ВХОДОВ */

/* Преобразовать вывод АЦП в напряжения */
inline void convert_adcs() {
  tvolt = adcs[0] * BASE_VOLTAGE / ADC_RESOLUTION;
  for(int i = 1; i < 6; i++) {
    volts[i-1] = adcs[i] * BASE_VOLTAGE / ADC_RESOLUTION;
  }
}

/* Прочитать и усреднить значение на входе АЦП */
int read_adc(int input) {
  int x = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    delay(READING_DELAY);
    x += analogRead(input);
  }
  return x / NUM_READINGS;
}

/* Прочитать значения АЦП */
inline void read_adcs() {
  
  int x = 0;

  adcs[0] = read_adc(VTOTALIN);
  adcs[1] = read_adc(V1IN);
  adcs[2] = read_adc(V2IN);
  adcs[3] = read_adc(V3IN);
  adcs[4] = read_adc(V4IN);
  adcs[5] = read_adc(V5IN);
  
}
