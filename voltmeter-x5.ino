
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


/* Аварийный сигнал */
#define VOLT_THRES 5 // Чувствительность отклонения для строки состояния, задается по входному напряжению до делителя
#define V_TOTAL_LIMIT 400 // Нижний предел сигнала суммарного напряжения, задается по входному напряжению до делителя
#define TONE_OUTPUT 3
#define TONE_LENGHT 500

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
#define VTOTALLED 4
#define VLEDSTART 5

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
  pinMode(VTOTALLED, OUTPUT);
  for(int i = 0; i < 5; i++) {
    pinMode(VLEDSTART + i, OUTPUT);
  }
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
  display.print(String("Ubat=" + String(tvolt, 0)));

  int i_max, i_min;
  float v_max = 0, v_min = 9999;
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
  display.print(String("U" + String(i_min + i_shift) + String("=") + String(volts[i_min], 0)));

  display.setCursor(0, 16);
  display.print(String("U" + String(i_max + i_shift) + String("=") + String(volts[i_max], 0)));

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
    if(volts[i] < average - VOLT_THRES) {
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

/* Преобразование входного напряжения с учетом преобразователей */
int v2section_v(float v, float k = 21.2, float b = 20.3) {
  // 0.33 25
  // 1.24 50
  // 2.52 75
  // 3.99 100
  // 4.80 125
  return k * v + b;
}

int v2total_v(float v) {
  // 1.61 400
  // 1.98 450
  // 2.38 500
  // 2.78 550
  // 3.20 600
  return 125 * v + 200;
}

float adc2v(int adc) {
  return adc * BASE_VOLTAGE / ADC_RESOLUTION;
}

/* Преобразовать вывод АЦП в напряжения */
inline void convert_adcs() {
  tvolt = v2total_v(adcs[0] * BASE_VOLTAGE / ADC_RESOLUTION);
  // Задаются коэффициенты наклона k и высоты b
  volts[0] = v2section_v(adc2v(adcs[1]), 21.2, 20.3);
  volts[1] = v2section_v(adc2v(adcs[2]), 21.2, 20.3);
  volts[2] = v2section_v(adc2v(adcs[3]), 21.2, 20.3);
  volts[3] = v2section_v(adc2v(adcs[4]), 21.2, 20.3);
  volts[4] = v2section_v(adc2v(adcs[5]), 21.2, 20.3);
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
