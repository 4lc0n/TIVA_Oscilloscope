# TIVA-Oszilloskop 
## Semesterarbeit Jürgen Markl MT2 SS 2020

### Über dieses Projekt
Dieses Projekt ist als Semesterarbeit im Fach Messtechnik 2 im Sommersemester 2020 entstanden. 
Es wurde mithilfe des Tiva Lauchpad, genauer dem TM4C123GXL Development Board ein Budget-Oszilloskop nach dem Vorbild des DSO138 entwickelt, welches eine Abtastrate von 1MSample/s und eine analoge Bandbreite von 75kHz aufweist. 
Die Software beinhaltet grundlegende Funktionen zur Signalanalyse wie Zoom-Funktion und horizontale Verschiebung mittels des Potentiometers, als auch verschiedene Einstellmöglichkeiten bezülgich des Eingangsspannungsteilers oder der Anzeigeoption. 
Grundlegende Messfunktionen wie Gleichanteil und Peak-to-Peak-Spannung sind implementiert. Diese können in der Zukunft erweitert werden durch beispielsweise eine Fourier-Analyse mit der Auswahlmöglichkeit verschiedener Fensterfunktionen. 




### Features
+ Eingangsspannung    +- 3.3V / +- 5V / +- 15V
+ Analoge Bandbreite: 75kHz
+ Abtastrate:         bis zu 1MSample/s
+ Messfunktionen:     mean, peak-to-peak
+ Display             1,8" RGB TFT Display (ST7735)
+ Darstellung: 	    dots (average), max/min

### How to

Um das Projekt zu kompilieren wird das Tivaware-Paket für den TM4C benötigt, welches von TI bezogen werden kann. 
Die Entwicklung fand mithilfe des Code Composer Studio 10 statt. 
Dort muss nach der Projektimportierung das Tivaware-Paket verlinkt werden: 
* Project > Properties > Linked Resources: New Variable: TIVAWARE_INSTALL /absoluter/pfad/zur/tivaware
* Build > Variables: Add TIVAWARE_INSTALL as Directory
* rechtsklick auf Project > Add files: /absoluter/pfad/zu/tivaware/driverlib/ccs/Debug/driverlib.lib
* Project > Properties > Build > ARM Compiler > Include options: ${TIVAWARE_INSTALL}


### Umsetzung
Der Displaytreiber für das ST7735 TFT Display wurden von Jonathan Valvano übernommen und erweitert, um den Anforderungen zu genügen

Um den ADC nicht in Software zu starten, was zu nicht-konstanten Abtastintervallen führen würde, wurde TIMER0A konfiguriert, dass dieser bei Ablauf die ADC-Wandlung startet.
Die Daten des ADCs werden sodann einzeln durch die µDMA-Einheit in einen Zwischenspeicher gebracht. Nach 128 Wandlungen wird der Zwischenspeicher gewechselt und die Daten durch den Prozessor in den entsprechenden FIFO Puffer geschrieben.

Der FIFO-Puffer ist nach Anleitung durch https://www.mikrocontroller.net/articles/FIFO#Code-Beispiel_2 implementiert und diese erweitert: 
* in den Funktionen muss ein Pointer auf den Stuct des Puffers übergeben werden, um zwei Puffer zu verwenden
* um eine Funktion erweitert, welche die Daten im Puffer überschreibt, wenn dieser voll ist (für den Pre-Trigger Buffer)

Verschiedenste Displayfunktionen wurden in display.h und ui.h implementiert, wobei Erstere für die Anzeige der Daten und zweitere für die Konfiguration zuständig ist.

Bisher implementierte Funktionen zur Analyse der Daten: 
* Mittelwertbildung
* Peak-to-Peak Spannung

Mögliche weitere Implementierungen: 
* FFT mit verschiedenen Fensterfunktionen
* Decoder für gängige digitale Übertragungsprotokolle (I2C, UART, RS232, RS485)

Da der µC über eine USB-Schnittstelle verfügt, wäre es auch denkbar diese zu Nutzen um mit hoher Geschwindigkeit Messdaten an einen PC zu übermitteln, um die Daten weiter zu verarbeiten (Anzeige, FFT, etc. )



### Schaltplan
Der Schaltplan ist als KiCAD-Projekt angehängt und für den LM324 optimiert. Jedoch ist dieser durch die Bandbreite eingeschränkt und somit keine optimale Wahl. 
Die Wahl eines leistungsfähigeren OP oder eine andere Hardwareimplementierung wäre hier sinnvoll. Ebenso soll die Triggereinheit von der analogen Eingangsbeschaltung getrennt werden, um Störsignale zu vermeiden.

