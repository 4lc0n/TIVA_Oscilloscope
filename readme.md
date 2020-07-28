# TIVA-Oszilloskop 
## Semesterarbeit Jürgen Markl MT2 SS 2020

### Über dieses Projekt
Dieses Projekt ist als Semesterarbeit im Fach Messtechnik 2 im Sommersemester 2020 entstanden. 
Es wurde mithilfe des Tiva Lauchpad, genauer dem TM4C123GXL Development Board ein Budget-Oszilloskop nach dem Vorbild des DSO138 entwickelt, welches eine Abtastrate von 1MSample/s und eine analoge Bandbreite von 75kHz aufweist. 
Die Software beinhaltet grundlegende Funktionen zur Signalanalyse wie Zoom-Funktion und horizontale Verschiebung mittels des Potentiometers, als auch verschiedene Einstellmöglichkeiten bezülgich des Eingangsspannungsteilers oder der Anzeigeoption. 
Grundlegende Messfunktionen wie Gleichanteil und Peak-to-Peak-Spannung sind implementiert. Diese können in der Zukunft erweitert werden durch beispielsweise eine Fourier-Analyse mit der Auswahlmöglichkeit verschiedener Fensterfunktionen. 




### Features
Eingangsspannung    +- 3.3V / +- 5V / +- 15V
Analoge Bandbreite: 75kHz
Abtastrate:         1MSample/s
Messfunktionen:     mean, peak-to-peak
Display             1,8" RGB TFT Display (ST7735)


### How to

Um das Projekt zu kompilieren wird das Tivaware-Paket für den TM4C benötigt, welches von TI bezogen werden kann. 
Die Entwicklung fand mithilfe des Code Composer Studio 10 statt. 
Dort muss nach der Projektimportierung das Tivaware-Paket verlinkt werden: 
* Project > Properties > Linked Resources: New Variable: TIVAWARE_INSTALL /absoluter/pfad/zur/tivaware
* Build > Variables: Add TIVAWARE_INSTALL as Directory
* rechtsklick auf Project > Add files: /absoluter/pfad/zu/tivaware/driverlib/ccs/Debug/driverlib.lib
* Project > Properties > Build > ARM Compiler > Include options: ${TIVAWARE_INSTALL}


