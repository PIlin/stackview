Diese Software errechnet den gesamten RAM Verbrauch einer Software (inkl. Stack!)
und erstellt eine Datei mit der Baumstruktur welche Funktion welche aufruft.
Dazu wird die vom Compiler erstellte ELF Datei mittels der ELFIO Lib eingelesen,
der Maschinencode, die Symboltabelle (welche Funktion an welcher Adresse steht)
und die globale Variablengr��e extrahiert.
Nun spielt die Software das Programm einmal komplett durch, betrachtet aber nur calls
und andere Stack manipulierende Befehle. So wird der maximale Stackverbrauch ermittelt.

Das funktioniert f�r "normale" Programme ganz gut, es gibt allerdings ein paar Situationen in
denen die Software scheitert (diese erzeugen eine Warning):
- icalls (Program Pointer): Da diese erst zur Laufzeit ermittelt werden, k�nnen diese nicht ausgef�hrt werden.
- rekursive Funktionen: Da diese eine Endlosschleife bewirken, werden auch diese nicht ausgef�hrt.
- nested Interrupts. Hier gilt �hnliches wie bei rekursiven Funktionen.

Solange keine der obigen Situationen auftreten, sollte die Berechnung exakt sein (falls ich nicht irgendeinen Fehler eingebaut habe.)

Diese Software ist nicht wirklich ausgereift! Eigentlich war es eine quick&dirty L�sung
um einen Fehler zu finden (den ich auch gefunden habe: Es lag tats�chlich am Stack!).

In der Software wurden etliche Annahmen getroffen (die einen Error erzeugen wenn sie nicht zutreffen, bzw. das Programm abst�rzt) wie z.B.
- eine Absch�tzung der Flashgr��e (notwendig f�r die rjmp/rcall Spr�nge �ber die Flashgrenze hinweg)
- der Code steht in der Section die .text hei�t
- der Einstiegspunkt ist die Funktion die in der Symboltabelle main hei�t
- bestimmte Befehlskombinationen mit dem der Compiler Stack belegt (in rx,SPL/SPH, sbiw rx, n)
- und noch ein paar weitere.

 
Zum Programm selbst:
Es ist ein ganz einfaches Kommandozeilentool, das als Paramter die elf Datei erwartet.
Anschlie�end arbeitet das Programm wenige Sekunden und sollte dann eine calltree.txt ausspucken und die ermittelten Werte anzeigen.
Allerdings schlie�t sich das Programm sofort wieder. Also entweder �ber eine DOS Box starten, oder die Werte am Ende der calltree.txt 
anschauen (da steht genau das gleiche).

Wenn alles gut gelaufen ist, sollte in dieser Datei nun sowas stehen (am Beispiel eines Hello World Programms mit UART):

Functions:

function: main                                                                            size: 115
  function: uart_init                                                                     size: 12
  end function                                                                            local stack:     2, total stack:    22
  function: uart_puts                                                                     size: 10
   function: uart_putc                                                                    size: 25
   end function                                                                           local stack:     4, total stack:    26
  end function                                                                            local stack:     2, total stack:    22
end function   


Interrupts:

function: __vector_1                                                                      size: 0
end function                                                                              local stack:     2, total stack:     2
... (hier stehen alle ungenutzten Vektoren mit einem eher zuf�lligen Namen)
function: __vector_1                                                                      size: 0
end function                                                                              local stack:     2, total stack:     2
function: __vector_8                                                                      size: 37
end function                                                                              local stack:     9, total stack:     9
function: __vector_1                                                                      size: 0
end function                                                                              local stack:     2, total stack:     2
... (hier stehen alle ungenutzten Vektoren mit einem eher zuf�lligen Namen)

Analysis complete: 0 errors, 0 warnings

flash usage (words):               588
global RAM usage (bytes):           16

maximum func. subroutine levels:     3
maximum func. stack (bytes):        26
maximum int. subroutine levels:      1
maximum int. stack (bytes):          9
nested interrupts:                   0
maximum nest. int. stack (bytes):    0

maximum total stack (bytes):        53
maximum total subroutine levels:     4
maximum total RAM usage (bytes):    69


Dies bedeutet jetzt, dass in der main Funktion zun�chst die uart_init() und anschlie�end die uart_puts(), die Wiederum die uart_putc() aufruft.
Pro Funktion werden 2Bytes an Stack verbraucht (f�r den Programcounter), zus�tzlich g�nnt sich die uart_putc weitere 2 Bytes an Stack.
Am Anfang jeder Funktion findet sich die Angabe size. Dies ist die Gr��e der Funktion in words. Am Ende jeder Funktion bedeutet die Angabe
local stack der Stackverbrauch der Funktion. total stack ist der Stackververbrauch der bisher aufgerufenen Funktionen (also der des gesamten
Funktionsbaumes). Hiervon wird der Maximalwert ermittelt.
F�r Interrupts gilt �hnliches:
Die ungenutzen Interrupts belegen 2Bytes an Stack. Der genutze UART Interrupt ben�tigt 9Bytes die zu dem Maximalwert des Funktionsstack addiert
werden, da die Interrupts jederzeit zuschlagen k�nnen (cli und sei in normalen Funktionen werden nicht ber�cksichtigt).
Sollte ein Interrupted nested sein, also innerhalb eines Interrupts werden die Interrupts wieder freigegeben, dann wird der lokale Stack dieses
Interrupts nochmal zum gesamten Verbrauch addiert. �hnliches gilt f�r rekursive Funktionen: Diese ruft sich insgesamt 1x selbst auf.



