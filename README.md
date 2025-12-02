
# INDOOR_AIR_QUALITY

### Členové týmu
Po domluvě byl každému účastníkovi projektu přidělen konkrétní senzor k nastudování a napsání kódu.
* Vítek Borovka - DHT12, I2C, display, dokumentace
* Jakub Kotačka  - GP2Y1010, ADC převodník, dokumentace
* Miroslav Vysloužil - MQ135, ADC převodník, dokumentace

---

## Základní vize projektu
Dýchání vzduchu je nedílnou součástí života. Občas se ale jeho kvalita může díky okolním vlivům zhoršit, a to může mít dopad na naše zdraví (únava, bolesti hlavy a jiné).
Základní myšlenka byla sestrojit platformu, která umožní vyhodnotit kvalitu ovzduší v místnosti. Konkrétně aby platforma měřila teplotu, vlhkost, kvalitu vzduchu a pevné částice. Cílem je tedy časté měření techto veličin s pomocí senzorů a zobrazení na displej. Pro řešení tohoto problému je využita vývojová deska Arduino UNO, která sesbírá, zpracuje a zobrazí data na displeji.
Na obrázku níže je představa zapojení platformy, její napájení a komunikace.

<img width="850" height="615" alt="image" src="https://github.com/user-attachments/assets/1ff3b3b7-9f76-4ded-84ee-4b81fe1b4a82" />

## Seznam hardwarových komponentů

### DHT12
DHT12 je digitální senzor určený pro měření teploty a vlhkosti. 

Senzor měří relativní vlhkost vzduchu v rozsahu 20 % až 95 % RH s přesností +-5 % RH.
Teplota se měří v rozsahu -20°C až 60°C s přesností +-0,5°C. Pracovní napětí senzoru je 2,7 V až 5,5 V.

Samotné měření vně senzoru probíhá pomocí NTC termistoru (zmenšuje svůj odpor s rostoucí teplotou), kapacitního čidla (dvě elektrody pokryté polymerní vrstvou, která absorbuje vodní páru a mění tak svou permitivitu a tím celkovou kapacitu mezi elektrodami) a interního mikrokontroléru (provádí přepočet hodnot odporu a kapacity na teplotu a vlhkost, formátuje výsledky do 5 bajtů).

Oproti starším modelům, jako je například DHT11, využívá DHT12 rozhraní I2C.

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
Typické napětí čistého vzduchu bývá kolem 1 V.
Koncentrace prachu se vypočítá přibližně:

PM = (V - V0)/0.005
	
Každých 0,005 V odpovídá přibližně 1 µg/m³ prachu.
Jedná se o zjednodušený lineární model podle datasheetu.

<img width="530" height="529" alt="image" src="https://github.com/user-attachments/assets/0efc3de2-6ac5-4986-a6cf-07d307c927eb" />


### Arduino UNO
Tato vývojová deska slouží jako centrální mozek celého projektu. 
#### Hlavní vlastnosti

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

### Schéma zapojení v nepájivém poli
<img width="956" height="833" alt="image" src="https://github.com/user-attachments/assets/2eb3ea66-55a3-4fd6-a787-f2f7b6ad951a" />




### Video


https://github.com/user-attachments/assets/60efba0b-d33e-45a5-a0d7-bd4f6daf292e




## Software
### Cíl

Hlavním požadavkem je měření kvality ovzduší, snímat hodnoty ze senzorů po určitém čase (v našem případě každých 5 vteřin) a výsledné hodnoty zobrazit na OLED display. Kvůli kontrole správného zobrazení na OLED display jsme se rozhodli hodnoty zobrazovat také pomocí UART na sériový monitor v počítači.

Jednotlivé senzory byly rozděleny mezi členy týmu. Každý člen se nejprve zaobíral problematikou svého senzoru a kódem samostatně. Následně jsme jednotlivé části kodů postupně zkompletovali. Problematikou zobrazování jsme se zabývali již společně.

<img width="498" height="769" alt="DE2 drawio" src="https://github.com/user-attachments/assets/c1ab9e70-3824-4cdd-ab52-16da34ebec31" />

### Postup vývoje

1. **Využití I²C sběrnice pro komunikaci mezi DHT12 a OLED displejem**

Jak již bylo zmíněno výše, oba tyto moduly využívají I²C sběrnici, díky čemuž je možné je připojit ke stejným dvěma vodičům (SDA a SCL). To výrazně zjednodušilo propojení i následnou komunikaci.

Nejprve jsme adresovali jednotlivá zařízení (SLAVE), aby mikrokontrolér (MASTER) rozlišil, se kterým modulem komunikuje:

 -DHT12: adresa 0x5C 
 
 -OLED displej: adresa 0x3C

<img width="1067" height="431" alt="Snímek obrazovky 2025-11-30 215106" src="https://github.com/user-attachments/assets/e68711dd-45c2-415f-bbe3-03ab2709a707" />


Dále jsme museli vyřešit problém získávání dat z DHT12, protože senzor poskytuje teplotu a vlhkost ve formě pěti bajtů:

 -bajt 0: integer vlhkosti
 
 -bajt 1: desetinná část vlhkosti
 
 -bajt 2: integer teploty
 
 -bajt 3: desetinná část teploty
 
 -bajt 4: kontrolní součet (checksum)

 Pro hodnoty jsme si vytvořili pole o velikosti 5 bajtů, kam se načítají data pomocí I²C čtení.

 <img width="561" height="131" alt="Snímek obrazovky 2025-11-30 215337" src="https://github.com/user-attachments/assets/757bf3a1-12ea-4bc2-be17-377d6ed75c12" />
  




## Prezentace – plakát
![Poster-(18-X-24-in)-(1) pdf](https://github.com/user-attachments/assets/9505bfdb-d042-4679-a5ae-6a0561603d46)

## Zdroje
DHT12:[DHT12_datasheet](https://github.com/koty974/indoor_air_quality/blob/main/sources/dht12_manual%20(2).pdf)
	  [Teploměr a vlhkoměr DHT12](https://navody.dratek.cz/navody-k-produktum/teplomer-a-vlhkomer-dht12.html)
	  [DHT12_description](https://electropeak.com/dht12-digital-temperature-and-humidity-sensor#:~:text=Description:%20The%20DHT12%20humidity%20sensor%20is%20an,digital%20conversion%20and%20produces%20the%20digital%20output.)

MQ-135:

GP2Y1010: https://global.sharp/products/device/lineup/data/pdf/datasheet/gp2y1010au_e.pdf

