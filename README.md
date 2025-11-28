
# INDOOR_AIR_QUALITY

### Členové týmu
-každý popíše svou část kódu a senzor
* Vítek Borovka (...) - I2C, display, DHT12
* Jakub Kotačka (...) - ADC převodník, gp2y1010
* Miroslav Vysloužil (...) - ADC převodník, MQ135 

---

## Fáze 1: Návrh
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


### Cíl

Naplánovat, navrhnout a odůvodnit projekt před samotnou implementací.

### Výstupy

1. **Formulace problému a návrh řešení**
   - Jasně popsat problém, který projekt řeší.
   - Vysvětlit, jak navržené řešení s využitím mikrokontroléru (MCU) tento problém řeší.

2. **Seznam hardwarových komponent**
   - Arduino UNO
   - doplní Mirek, má u sebe
   - Sensor DHT12 pro měření teploty a vlhkosti
   - ... (další komponenty doplnit)
   - Vysvětlení, proč je každá komponenta použita a jaký má účel.

3. **Návrh softwaru**
   - Systémové **blokové schéma**, **vývojové diagramy** nebo **pseudokód** popisující plánovanou logiku a tok programu.

---
## Fáze 2: Konstrukce (Vývoj prototypu)
- fotografie vývoje- popsat problémy během vývoje
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



