# Controllo Altitudine Drone

Autore: Matteo Onger 

Descrizione: Il progetto punta a recuperare il telaio e i motori di un drone già esistente ma danneggiato, sostituendone la circuiteria di controllo e programmandola     affinché, tramite messaggi MQTT, sia possibili controllare remotamente il quadrirotore. In particolare, sfruttando un sensore ad ultrasuoni posto sotto la                scocca del drone, sarà possibile impostare una quota obiettivo che il veivolo, controllato dal microcontrollore, punterà a raggiungere. Sempre attraverso                messaggi MQTT sarà possibile ricevere informazioni sullo stato del drone o forzare un atterraggio graduale mediato del MCU.

### Hardware:
Già presenti:
* Telaio quadrirotore Nine Eagles Galaxy Visitor 6 Pro
* 4 x Motori 8520D [[doc]](https://www.ricmotor.com/details/8520-coreless-motor)

Aggiunti:
* Wemos D1 Mini [[doc]](https://www.wemos.cc/en/latest/d1/d1_mini.html)
* 2 x Dual H Bridge L298H [[doc]](https://github.com/MatteoOnger/Drone/blob/main/Documents/Specifiche.md) (ogni ponte H controlla due motori)
* Sensore ultrasuoni HC-SR04 [[doc]](https://www.makerslab.it/sensore-di-distanza-ad-ultrasuoni-hc-sr04-con-arduino/)
* Connettori Micro JST 1.25 mm
* Jumper
* Batterie LiPo 3.7V

### Librerie usate:
* ESP8266WiFi.h [[doc]](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html)
* PubSubClient.h [[doc]](https://pubsubclient.knolleary.net/api)

### Struttura messaggi MQTT:
E' possibile inviare al drone tre diversi dipi di messaggio:
1. 'S' : comanda al drone di atterrare.
2. 'I' : chiede al drone di fornire informazioni circa il suo stato attuale.
3. 't:trg:esp' : imposta una quota obiettivo *trg* (intero espresso in cm) con un margine di errore *eps* (intero espresso in cm).

### Note:
1. Per via dello spazio limitato è necessario tagliare su misura e saldare i jumper usati per connettere i vari componenti  
