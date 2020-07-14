# Semesteraufgabe Advanced Python

## Thema: DSP mit Python

### Vorbereitung DSP
* Das Programm **genWaveforms.c** erstellt die für die DSP benötigten Waveforms.
* Dabei werden für jeden Channel in den jeweiligen Ordner 128 Waveforms mit gleichem Nutzsignal, aber unterschiedlichem Rauschen erstellt.
* Die Waveforms können mit dem Befehl **make waveforms** erstellt werden.
* Nicht wundern: Das kompilierte Programm wird am Ende auch gelöscht, da es nach der Generierung der Waveform nicht mehr benötigt wird.
* **make clean** löscht die Waveforms wieder.

### Ablauf DSP
* Die erstellen Waveforms werden anschließen mit den Programmen in Octave, Python und C verarbeitet. Dabei werden folgende Techniken genutzt:
  * Averaging (Aufsummieren und anschließendes normieren der Waveforms als Vector) (**Schwerpunkt numpy**)
  * Upsampling (mit fft) (**Schwerpunkt scipy**)
  * Signallaufzeit anpassen (mit cross correlation) (**Schwerpunkt scipy**)
  * Ausgabe der Ergebnisse (**Schwerpunkt Matplotlib**)

* Zusätzlich werden die Verarbeitungszeiten gemessen.
