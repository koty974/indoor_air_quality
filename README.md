
# INDOOR_AIR_QUALITY

### Členové týmu
Po domluvě byl každému účastníkovi projektu přidělen konkrétní senzor k nastudování a napsání kódu.
* Vítek Borovka - DHT12, I2C, display, dokumentace
* Jakub Kotačka  - gp2y1010, ADC převodník, dokumentace
* Miroslav Vysloužil - MQ135, ADC převodník, dokumentace

---

## Základní vize projektu

<img width="850" height="615" alt="image" src="https://github.com/user-attachments/assets/1ff3b3b7-9f76-4ded-84ee-4b81fe1b4a82" />

## Seznam hardwarových komponentů

### DHT12

<img width="408" height="336" alt="image" src="https://github.com/user-attachments/assets/8258d98e-c46c-4049-a049-59689eefd782" />

### MQ-135
MQ-135 je senzor pro detekci kvality vzduchu.

Dokáže detekovat amoniak, oxid dusíku, benzen, alkohol, kouř a další škodlivé látky. MQ-135 nedokáže poznat konkrétní plyn. Reaguje na více látek najednou, takže výsledek je jen orientační.

Uvnitř senzoru je malá topná spirála, která ohřívá vrstvu oxidu cíničitého. Při výskytu nějakého plynu se změní odpor této vrstvy, a tím se mění i napětí na výstupním analogovém pinu.

Ten připojíme na AD převodník A0 mikrokontroleru, který následně analogovou hodnotu převede na digitální v rozsahu 0-1023. Podle této hodnoty můžeme určit kvalitu okolního vzduchu.

 **Očekávané hodnoty**


Čistý venkovní vzduch				150–200

Průměrný vnitřní prostor			250–350

Znečištěná oblast					400–600+

Blízko alkoholu/benzínu				700–900+

<img width="580" height="405" alt="image" src="https://github.com/user-attachments/assets/7887a600-30de-4814-9e15-83fa93a4c327" />

### GP2Y1010
GP2Y1010 je optický senzor prachových částic, který měří koncentraci prachu (PM) v ovzduší pomocí rozptylu infračerveného světla.
Uvnitř senzoru se nacházejí dvě hlavní součásti:
1. Infračervená LED
Senzor obsahuje IR LED, která osvětluje vnitřní měřicí komoru. Pokud se ve vzduchu nacházejí prachové částice, dochází k rozptylu světla, které dopadá na detektor.
LED nesmí být rozsvícena trvale – musí být řízena přesným časováním doporučeným výrobcem:
280 µs LED ON
40 µs LED ON + měření
9,7 ms LED OFF
Tato sekvence se opakuje v cyklu a zajišťuje správné a stabilní měření bez přehřívání LED.
2. Fotodioda (detektor)
Fotodioda snímá množství světla odraženého od prachových částic.
Čistý vzduch → nízké napětí (cca 1,0–1,9 V)
Prašné prostředí → více rozptýleného světla → vyšší napětí
Senzor tedy poskytuje analogový výstup, který odpovídá koncentraci prachu.

Stavový automat řízený Timer0
Knihovna využívá přerušení Timer0 Overflow, které nastává každých 16 µs.
V přerušení běží stavový automat se třemi stavy:
Stav 0 – LED ON (≈280 µs)
LED se rozsvítí a po 18 tazích Timer0 (≈288 µs) přechází do dalšího stavu.
Stav 1 – Měření ADC (≈40 µs)
LED stále svítí.
Po 1 tiknutí se provede:
výběr kanálu ADC1
spuštění konverze ADC
uložení výsledku do last_raw
Po dvou tikách (≈32 µs) se LED zhasne a pokračuje se do dalšího stavu.
Stav 2 – LED OFF (≈9,7 ms)
LED zůstává vypnutá po dobu 605 tiků Timer0 (≈9,68 ms).
Poté se cyklus opakuje.

DC → napětí – gp2y1010_adc_to_voltage()

Výpočet napětí z 10bit ADC:

𝑉=(ADC*5)/1023

5V je referenční napětí (AVcc).

Napětí → koncentrace prachu – gp2y1010_voltage_to_density()
Typické napětí čistého vzduchu bývá kolem 1,9 V.
Koncentrace prachu se vypočítá přibližně:

PM = (V - V0)/0.005
	
Každých 0,005 V odpovídá přibližně 1 µg/m³ prachu.
Jedná se o zjednodušený lineární model podle datasheetu.

<img width="530" height="529" alt="image" src="https://github.com/user-attachments/assets/0efc3de2-6ac5-4986-a6cf-07d307c927eb" />


### Arduino UNO
Tato vývojová deska slouží jako centrální mozek celého projektu. Na desce se nachází mikrokontroler ATmega328P s velkoým počtem základních periferií.

## Hlavní vlastnosti

Mikrokontrolér: ATmega328P

Napájení: 5 V (USB) nebo 7–12 V externě

Digitální piny: 14 (z toho 6 PWM výstupů)

Analogové vstupy: 6

Flash paměť: 32 KB (z toho 0,5 KB pro bootloader)

RAM: 2 KB

Komunikace: UART, SPI, I2C

Rozhraní: USB-B pro programování, napájení a komunikaci s PC
<img width="631" height="431" alt="image" src="https://github.com/user-attachments/assets/a8c9721f-2292-474f-a243-1088ef4c2037" />

### OLED displej
OLED displej funguje na bázi tzv. organických diod, což jsou součástky schopné 
generovat světelné záření v případě, že na ně působí elektrické pole. Do 
obrazovky se nijak nemontují, nýbrž se společně s dalšími vrstvami nanášejí na
skleněný či jiný podklad. Námi použitý displej má malé rozměry, komunikaci s I2C a pracovní napájecí napětí 3,3/5V.
<img width="440" height="440" alt="image" src="https://github.com/user-attachments/assets/5ab46378-585c-4155-8a8b-92ad5be7220b" />

## Zapojení
### Schéma zapojení

<img width="944" height="705" alt="image" src="https://github.com/user-attachments/assets/8318c876-3d57-438d-a702-1c8154eba07a" />

### Schéma zapojení v nepájivém poly
<img width="945" height="709" alt="image" src="https://github.com/user-attachments/assets/11fa0962-498c-491a-9682-d716b68f81e2" />

### Video




### Cíl

Naplánovat, navrhnout a odůvodnit projekt před samotnou implementací.

### Výstupy

1. **Formulace problému a návrh řešení**
   - Jasně popsat problém, který projekt řeší.
   - Vysvětlit, jak navržené řešení s využitím mikrokontroléru (MCU) tento problém řeší.


3. **Návrh softwaru**
   - Systémové **blokové schéma**, **vývojové diagramy** nebo **pseudokód** popisující plánovanou logiku a tok programu.

---
## Fáze 2: Konstrukce (Vývoj prototypu)
- fotografie vývoje- popsat problémy během vývoje
- 
### Cíl
Implementovat a otestovat funkční prototyp na základě schváleného návrhu
- video funkčního prototypu
   

### Poznámky
- Týmy mohou využívat univerzitní laboratoře a zařízení pro práci s hardwarem.
- Konstrukce obvodu může zahrnovat pasivní i aktivní komponenty na breadboardu nebo na vlastní PCB (pokud je k dispozici).
- popíše Kuba odpory a kondíky

### Výstupy

1. **Video demonstrace prototypu**
   - Krátké **max. 3min video** ukazující funkcionalitu prototypu.
   - Srozumitelně vysvětlit funkce systému, způsob ovládání a chování.

2. **Odevzdání zdrojového kódu**
   - Dobře okomentovaný C/C++ kód (Arduino) nahraný na GitHub.
   - Zdůraznit klíčové funkce, algoritmy a případné vlastní knihovny.

3. **Technická dokumentace**
   - Bloková schémata
   - Schémata zapojení
   - Odůvodnění hlavních rozhodnutí v návrhu hardwaru i softwaru

4. **Prezentace – plakát (A3 nebo větší)**
   - Vizualizace shrnující:
     - Koncept projektu a jeho motivaci
     - Návrh systému a funkčnost
     - Přínos, využití a potenciální dopady



