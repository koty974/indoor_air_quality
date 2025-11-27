// -- Includes -------------------------------------------------------
// Základní AVR knihovny
#include <avr/io.h>         // Definice registrů, portů a bitů pro AVR
#include <avr/interrupt.h>  // Makra pro povolení/zablokování přerušení
#include "timer.h"          // Vlastní knihovna pro konfiguraci timerů
#include <twi.h>            // I2C (TWI) komunikace
#include <uart.h>           // UART komunikace (Peter Fleury)
#include <stdio.h>          // sprintf a snprintf pro formátovaný text
#include <oled.h>           // OLED knihovna
#include "gp2y1010.h"       // Sharp GP2Y1010 senzor prachu

// -- Defines --------------------------------------------------------
// I2C adresa DHT12 senzoru
#define DHT_ADR      0x5c
#define DHT_HUM_MEM  0      // Memory adresa pro vlhkost
#define DHT_TEMP_MEM 2      // Memory adresa pro teplotu

// ADC pin pro MQ135 senzor
#define MQ135_PIN 0         // ADC0 na Arduino Uno

// -- Global variables -----------------------------------------------
// Flag indikující, že je potřeba aktualizovat data na UART
volatile uint8_t flag_update_uart = 0;

// Flag indikující, že je potřeba aktualizovat OLED
volatile uint8_t flag_update_oled = 0;

// Pole pro uložení hodnot z DHT12 senzoru
volatile uint8_t dht12_values[5]; // [0]=hum int, [1]=hum dec, [2]=temp int, [3]=temp dec, [4]=checksum

// Hodnoty MQ135 senzoru
volatile uint16_t mq135_value = 0;    // Raw ADC hodnota
volatile uint16_t air_quality = 0;    // Vypočtená kvalita vzduchu

// Hodnoty GP2Y1010 senzoru prachu
volatile uint16_t dust_raw = 0;       // Raw ADC hodnota prachu
volatile float dust_voltage = 0;      // Přepočtená napěťová hodnota
volatile float dust_density = 0;      // Přepočtená hustota prachu v ug/m3

// Struktura senzoru GP2Y1010
GP2Y1010 dust = {
    .ledPin = 2,    // PD2 digitální pin pro LED senzoru
    .analogPin = 1  // ADC1 pro měření napětí z senzoru
};

// ------------------ OLED SETUP ------------------
void oled_setup(void)
{
    oled_init(OLED_DISP_ON);  // Inicializace OLED a zapnutí displeje
    oled_clrscr();             // Vymazání celé obrazovky

    // Nadpis velkým písmem
    oled_charMode(DOUBLESIZE);
    oled_puts("KVALITA VZDUCHU");
    oled_charMode(NORMALSIZE); // Návrat na normální velikost písma

    // Popisky jednotlivých hodnot
    oled_gotoxy(0, 2);
    oled_puts("MQ135 ADC:");
    oled_gotoxy(0, 3);
    //oled_puts("MQ135 kvalita:");
    //oled_gotoxy(0, 4);
    oled_puts("Uroven:");
    oled_gotoxy(0, 4);
    oled_puts("Teplota [°C]:");
    oled_gotoxy(0, 5);
    oled_puts("Vlhkost [%%]:");  // %% je pro zobrazení %
    oled_gotoxy(0, 6);
    oled_puts("Prach [ug/m3]:");

    oled_display();  // Kopíruje buffer do OLED RAM a zobrazí obsah
}

// -------- ADC INITIALIZATION ---------
void adc_init(void)
{
    // Nastavení referenčního napětí ADC na AVcc (5V)
    ADMUX |= (1 << REFS0);

    // Nastavení vstupního kanálu ADC0
    ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));

    // Zapnutí ADC a nastavení předděličky na 128 (16 MHz / 128 = 125 kHz)
    ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

// Funkce pro čtení ADC hodnoty z libovolného kanálu
uint16_t adc_read(uint8_t channel)
{
    // Výběr ADC kanálu (0-7)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // Start konverze
    ADCSRA |= (1 << ADSC);

    // Čekání na dokončení konverze
    while (ADCSRA & (1 << ADSC));

    // Vrací naměřenou hodnotu
    return ADC;
}

// Funkce pro určení textového popisu kvality vzduchu podle MQ135
const char* get_quality_level(uint16_t value)
{
    if (value < 300) return "VYNIKAJICI";
    if (value < 500) return "DOBRA";
    if (value < 700) return "STREDNI";
    if (value < 900) return "SPATNA";
    return "VELMI SPATNA";
}

// -------- TIMER1 (1s) INITIALIZATION --------
void timer1_init(void)
{
    tim1_ovf_1sec();   // Nastavení Timer1 pro přetečení každou 1 sekundu
    tim1_ovf_enable();  // Povolení přerušení při přetečení Timer1
}

/*void timer0_init(void)
{
    tim0_ovf_16us();   // Nastavení Timer1 pro přetečení každou 1 sekundu
    tim0_ovf_enable();  // Povolení přerušení při přetečení Timer1
}*/

// ------------------------ MAIN ------------------------
int main(void)
{
    // Buffery pro formátovaný text
    char uart_msg[10];    // Pro UART výpisy
    char oled_msg[10];    // Pro OLED zobrazení hodnot

    // Inicializace periferií
    twi_init();                                   // I2C
    uart_init(UART_BAUD_SELECT(115200, F_CPU));  // UART 115200 baud
    adc_init();                                   // ADC
    oled_setup();                                 // OLED
    gp2y1010_init(&dust);                         // GP2Y1010 senzor prachu

    // Timer0 pro GP2Y1010 LED řízení (overflow každých 16us)
    tim0_ovf_16us();
    tim0_ovf_enable();

    sei();               // Povolení globálních přerušení
    timer1_init();       // Inicializace Timer1 pro periodické měření
    //timer0_init(); 

    while (1)
    {
        // ---------------- UPDATE OLED ----------------
        if (flag_update_oled == 1)
        {
            // Výpočet "kvality vzduchu" pro MQ135
            air_quality = mq135_value / 10;

            // Vymazání staré ADC hodnoty a zobrazení nové
            oled_gotoxy(12,2);
            oled_puts("     ");                      // Vymazání starých číslic
            oled_gotoxy(12,2);
            sprintf(oled_msg, "%4d", mq135_value);  // Formátování na 4 znaky
            oled_puts(oled_msg);

            // Vymazání staré kvality vzduchu a zobrazení nové
            //oled_gotoxy(15,3);
            //oled_puts("     ");
            //oled_gotoxy(15,3);
            //sprintf(oled_msg, "%3d", air_quality);
            //oled_puts(oled_msg);

            // Vymazání starého textu a zobrazení slovní úrovně kvality
            oled_gotoxy(8,3);
            oled_puts("             ");
            oled_gotoxy(8,3);
            oled_puts(get_quality_level(air_quality));

            // Teplota DHT12
            oled_gotoxy(13,4);
            oled_puts("        "); 
            oled_gotoxy(13,4);
            sprintf(oled_msg, "%u.%u C", dht12_values[2], dht12_values[3]);
            oled_puts(oled_msg);

            // Vlhkost DHT12
            oled_gotoxy(13,5);
            oled_puts("        ");
            oled_gotoxy(13,5);
            sprintf(oled_msg, "%u.%u %%", dht12_values[0], dht12_values[1]);
            oled_puts(oled_msg);

            // Prach GP2Y1010
            oled_gotoxy(14,6);
            oled_puts("        ");
            oled_gotoxy(14,6);

            uint16_t dust_int = (uint16_t)(dust_density);
            uint16_t dust_dec = (uint16_t)((dust_density - dust_int) * 10);

            sprintf(oled_msg, "%u.%u", dust_int, dust_dec);
            oled_puts(oled_msg);

            //sprintf(oled_msg, "%.1f", dust_density);
            //oled_puts(oled_msg);

            // Aktualizace OLED
            oled_display();

            // Reset flagu
            flag_update_oled = 0;
        }

        // ---------------- UPDATE UART ----------------
        if (flag_update_uart == 1)
        {
            // Teplota
            sprintf(uart_msg, "Teplota: %u.%u C\r\n", dht12_values[2], dht12_values[3]);
            uart_puts(uart_msg);

            // Vlhkost
            sprintf(uart_msg, "Vlhkost: %u.%u %%\r\n", dht12_values[0], dht12_values[1]);
            uart_puts(uart_msg);

            // MQ135 raw hodnota a kvalita
            sprintf(uart_msg, "MQ135 tt=%u kvalita=%u\r\n", mq135_value, air_quality);
            uart_puts(uart_msg);

            // Prach GP2Y1010
            //sprintf(uart_msg, "Prach: %.1f ug/m3\r\n\r\n", dust_density);
            //uart_puts(uart_msg);

            // Prach GP2Y1010 – bezpečné pevné formátování
            uint16_t dust_int = (uint16_t)dust_density;
            uint16_t dust_dec = (uint16_t)((dust_density - dust_int) * 10);

            sprintf(uart_msg, "Prach: %u.%u ug/m3\r\n\r\n", dust_int, dust_dec);
            uart_puts(uart_msg);

            // Reset flagu
            flag_update_uart = 0;
        }
    }

    return 0; // Program se nikdy nedostane sem
}

// ------------------------ ISR TIMER1 ------------------------
// Timer1 přerušení každou 1 sekundu
ISR(TIMER1_OVF_vect)
{
    static uint8_t counter = 0;  // Počítadlo přetečení Timer1
    counter++;

    // Každých 5 sekund provést měření
    if (counter >= 5)
    {
        counter = 0;

        // Čtení MQ135
        mq135_value = adc_read(MQ135_PIN);

        // Čtení DHT12 přes I2C (5 bytů)
        twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 5);

        // Čtení GP2Y1010
        dust_raw = gp2y1010_read_raw(&dust);
        dust_voltage = gp2y1010_adc_to_voltage(dust_raw);
        dust_density = gp2y1010_voltage_to_density(dust_voltage);

        // Nastavení flagů pro hlavní smyčku
        flag_update_oled = 1;
        flag_update_uart = 1;
    }
}

