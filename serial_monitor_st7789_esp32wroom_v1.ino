/*  APROVADO
/*
Toda linha recebida (terminada por \n) é exibida no TFT.
com @ o conteúdo anterior é apagado para dar lugar à nova mensagem.
com > quebra linha

Display ST7789   →   ESP32-WROOM
--------------------------------
VCC              →   3.3V
GND              →   GND
SCL (Clock)      →   GPIO18
SDA (Data/MOSI)  →   GPIO23
CS               →   GPIO5
DC               →   GPIO16
RES              →   GPIO17
BLK (Backlight)  →   GPIO4 (PWM via LEDC)

compilado com board 'ESP32 Dev Module'

*/
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// -------------------- Definição de pinos ESP32 --------------------
#define TFT_CS   5    // Chip select
#define TFT_RST  17   // Reset
#define TFT_DC   16   // Data/Command
#define TFT_MOSI 23   // SPI MOSI
#define TFT_SCLK 18   // SPI Clock

#define BLpin    4    // Backlight (ON/OFF)
#define pGREEN   25   // LED Verde
#define pRED     26   // LED Vermelho
#define pBLUE    27   // LED Azul

// -------------------- Inicialização do display --------------------
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// -------------------- Buffer de entrada --------------------
const int MAX_MESSAGE_LENGTH = 168;
char input_buffer[MAX_MESSAGE_LENGTH];
int buffer_index = 0;
const char clearCommand = '@';
const int larguraLinha = 17;

// -------------------- Funções auxiliares --------------------
int min_val(int a, int b) {
  return (a < b) ? a : b;
}

void exibirBuffer(const char* texto) {
  int texto_len = strlen(texto);
  int pos = 0;

  while (pos < texto_len) {
    int segment_length;
    bool manual_break = false;

    const char* manual_break_ptr = strchr(texto + pos, '>');
    if (manual_break_ptr != NULL) {
      int break_index = manual_break_ptr - texto;
      int length_until_break = break_index - pos;

      if (length_until_break > 0 && length_until_break <= larguraLinha) {
        segment_length = length_until_break;
        manual_break = true;
      } else {
        segment_length = larguraLinha;
      }
    } else {
      segment_length = larguraLinha;
    }

    segment_length = min_val(segment_length, texto_len - pos);

    char trecho[larguraLinha + 1];
    strncpy(trecho, texto + pos, segment_length);
    trecho[segment_length] = '\0';

    tft.println(trecho);
    pos += segment_length;

    if (manual_break) {
      pos++;
    }
  }
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);

  // Configuração dos pinos como saída
  pinMode(BLpin, OUTPUT);
  pinMode(pRED, OUTPUT);
  pinMode(pGREEN, OUTPUT);
  pinMode(pBLUE, OUTPUT);

  // Liga o backlight e o LED vermelho por padrão
  digitalWrite(BLpin, HIGH);
  digitalWrite(pRED, HIGH);
  digitalWrite(pGREEN, LOW);
  digitalWrite(pBLUE, LOW);

  // Inicialização do display
  tft.init(240, 320, SPI_MODE0); // resolução do ST7789
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);

  tft.println("ESP32-WROOM");
  tft.println(" ");
  tft.setTextColor(ST77XX_GREEN);
  tft.println("BAUDRATE: 115200");
  tft.println("use '@' para limpar a tela");
  tft.println("use '>' para quebrar linha");
  tft.print("caract.largura = ");
  tft.println(larguraLinha);
  tft.println(" ");
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_RED);
  tft.print("Pronto para receber...");

  Serial.println("Ola!");
  Serial.println("Pronto para exibir dados. Envie por aqui.");
  Serial.flush();
}

// -------------------- Loop --------------------
void loop() {
  while (Serial.available()) {
    delay(2);
    char c = Serial.read();

    if (c == '\n') {
      input_buffer[buffer_index] = '\0';

      if (buffer_index == 1 && input_buffer[0] == clearCommand) {
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(0, 10);
        buffer_index = 0;
      } else {
        exibirBuffer(input_buffer);
        buffer_index = 0;
      }

      if (strcmp(input_buffer, "[RED_INV]") == 0) {
        tft.fillScreen(ST77XX_RED);
        tft.setTextColor(ST77XX_BLACK);
        digitalWrite(pRED, LOW); // apaga LED vermelho
        tft.setCursor(0, 10);
        buffer_index = 0;
      }

    } else {
      if (buffer_index < MAX_MESSAGE_LENGTH - 1) {
        input_buffer[buffer_index] = c;
        buffer_index++;
      }
    }
  }
}
