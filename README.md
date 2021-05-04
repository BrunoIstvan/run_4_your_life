## Run4YourLife - Display 16x2 - Buzzer e Joystick - Arduino Uno ou Mega

Este exemplo de aplicação irá mostrar um jogo do tipo Run4YourLife no display 16x2, informações nas 2 linhas, efeitos sonoros no Buzzer e Joystick para mover o carro.

----

## Pinagem:

### Display:
|Terminais|Arduino|
|-|-|
|VSS|GND|
|VDD|5V|
|V0|Pino do meio do Potenciômetro|
|RS|D7|
|RW|GND|
|E|D6|
|D0|-|
|D1|-|
|D2|-|
|D3|-|
|D4|D5|
|D5|D4|
|D6|D3|
|D7|D2|
|A|5V|
|K|Terminal Resistor 220Ohms -> GND|

----- 

### Botão:
|Terminais|Arduino|
|-|-|
|1|D13|
|2|GND|

----- 

### Buzzer:
|Terminais|Arduino|
|-|-|
|+|D12|
|-|Terminal Resistor 100Ohms -> GND|

----- 

### Potenciômetro:
|Terminais|Arduino|
|-|-|
|1|5V|
|2|Ligado ao V0 do Display 16x2|
|3|GND|

----

### Joystick:
|Terminais|Arduino|
|-|-|
|GND|GND|
|+5V|5V|
|VRX|A1|
|VRY|A0|

----
