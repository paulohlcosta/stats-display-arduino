/*  em testes

1.3: INCLUIR RGB, INCLUIR PWM BACKLIGHT*/

/*
O Arduino Nano escuta a porta serial.
Toda linha recebida (terminada por \n) é exibida no tft.
com @ o conteúdo anterior é apagado para dar lugar à nova mensagem.
com > quebra linha

GND → Arduino GND 
VCC → Arduino 3.3V (use a level shifter if your Arduino outputs 5V) 
SCL (Clock) → Arduino D13 
SDA (Data) → Arduino D11 
RES (Reset) → Arduino D8 
DC (Data/Command) → Arduino D9 
BLK (Backlight) → Leave unconnected or connect to GND to turn off backlight.
CS D10

*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
// Define pins
#define TFT_CS 10 // Chip select pin (if applicable)
#define TFT_RST 8 // Reset pin
#define TFT_DC 9 // Data/Command pin
#define BLpin 3 //Backlight PWM
#define pGREEN 4 //LED 
#define pRED 5 //LED PWM
#define pBLUE 6 //LED PWM

// Initialize ST7789 object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Alocação estática de memória para o buffer (MUITO MELHOR que String no Nano)
const int MAX_MESSAGE_LENGTH = 168;    // Tamanho máximo do buffer (168 bytes)
char input_buffer[MAX_MESSAGE_LENGTH]; // Buffer de entrada
int buffer_index = 0;                  // Índice atual do buffer
const char clearCommand = '@';                             // comando para limpar tela
const int larguraLinha = 17;   // número máximo de caracteres por linha
int brightness = 150;  // how bright the LED is

// Função auxiliar para encontrar o menor valor (necessário para o word wrap)
int min_val(int a, int b) {
  return (a < b) ? a : b;
}

// Adaptação da função para usar C-style string (const char*)
void exibirBuffer(const char* texto) {
  int texto_len = strlen(texto);
  int pos = 0; // Posição atual de leitura no texto

  while (pos < texto_len) {
    int segment_length;
    bool manual_break = false;
    
    // 1. Procura pela quebra manual ('>') a partir da posição atual
    const char* manual_break_ptr = strchr(texto + pos, '>');
    
    if (manual_break_ptr != NULL) {
      // Calcula o índice e o comprimento até o delimitador '>'
      int break_index = manual_break_ptr - texto;
      int length_until_break = break_index - pos;
      
      if (length_until_break > 0 && length_until_break <= larguraLinha) {
        // Quebra manual válida e dentro do limite da linha
        segment_length = length_until_break;
        manual_break = true;
      } else {
        // Quebra manual existe, mas está muito longe (> larguraLinha)
        segment_length = larguraLinha;
      }
    } else {
      // Não há quebra manual, usa a largura máxima da linha
      segment_length = larguraLinha;
    }
    
    // Garante que o comprimento do segmento não exceda o texto restante
    segment_length = min_val(segment_length, texto_len - pos);

    // 2. Extrai e imprime o segmento usando strncpy
    char trecho[larguraLinha + 1]; // +1 para o terminador nulo
    strncpy(trecho, texto + pos, segment_length);
    trecho[segment_length] = '\0'; // Adiciona o terminador nulo
    
    tft.println(trecho);
    
    pos += segment_length;
    
    // Se houve quebra manual, avança a posição para pular o caractere '>'
    if (manual_break) {
      pos++; 
    }
  }
}

void setup() {
  pinMode(BLpin, OUTPUT);
  pinMode(pGREEN, OUTPUT);
  pinMode(pRED, OUTPUT);
  pinMode(pBLUE, OUTPUT);
  digitalWrite(pRED, HIGH);
  analogWrite(BLpin, brightness);
  Serial.begin(115200);           // Inicializa comunicação serial
  // O nome do arquivo não pode ser obtido facilmente no ambiente Arduino sem String,
  // mas como só é usado em setup, manteremos o uso de String para simplificar a exibição inicial.
  String sketchName = String(__FILE__); 

  tft.init(240, 320); // Inicializa com a resolução do display
  tft.setRotation(1); // Define a orientação da tela (Horizontal)
  tft.fillScreen(ST77XX_BLACK); // Limpa a tela
  
  // Configurações de texto inicial
  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  
  tft.println(sketchName);
  tft.println(" ");
  tft.setTextColor(ST77XX_GREEN);
  tft.println("BAUDRATE: 115.200");
  tft.println("use '@' para limpar a tela");
  tft.println("use '>' para quebrar linha");
  tft.print("caract.largura = ");
  tft.println(larguraLinha); // Imprime o valor inteiro
  tft.println(" ");
  tft.setTextSize(3);
//  tft.setTextColor(ST77XX_WHITE);
  tft.setTextColor(ST77XX_RED);
  tft.print("Pronto para receber...");

  Serial.println("Ola!");
  Serial.println("Nome do sketch: ");
  Serial.println(sketchName);
  Serial.println("Pronto para exibir dados. Envie por aqui.");
  Serial.flush();

}

void loop() {
  while (Serial.available()) {
    delay(2);
    char c = Serial.read();

    // 1. Se receber caractere de nova linha, processa o buffer acumulado
    if (c == '\n') {
      
      // Termina a string C-style para garantir que seja tratada corretamente
      input_buffer[buffer_index] = '\0';
      
      // Verifica se o comando é limpar a tela (apenas o caractere de comando)
      if (buffer_index == 1 && input_buffer[0] == clearCommand) {
        tft.fillScreen(ST77XX_BLACK); // Limpa a tela
        tft.setCursor(0, 10);
        buffer_index = 0; // Limpa o índice
      } else {
        exibirBuffer(input_buffer);
        buffer_index = 0;
      }
      
      // comando fundo vermelha
      if (input_buffer == "[RED_INV]") {
        tft.fillScreen(ST77XX_RED); // Limpa a tela
        tft.setTextColor(ST77XX_BLACK);
        digitalWrite(pRED, LOW);
        tft.setCursor(0, 10);
        buffer_index = 0; // Limpa o índice
      }

    } else {
      // 2. Acumula o caractere no buffer
      
      // Evita o estouro de buffer (buffer overflow)
      if (buffer_index < MAX_MESSAGE_LENGTH - 1) {
        input_buffer[buffer_index] = c;
        buffer_index++;
      }
      // Se o buffer estiver cheio, o caractere é silenciosamente descartado.
    }
  }
}
