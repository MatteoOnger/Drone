# Controllo Altitudine Drone

Autore: Matteo Onger 

Descrizione: Il progetto punta a recuperare il telaio e i motori di un drone già esistente ma danneggiato, sostituendone la circuiteria di controllo e programmandola affinché, tramite messaggi MQTT, sia possibile controllare da remoto il quadrirotore. In particolare, una volta acceso, sfruttando un sensore ad ultrasuoni posto sotto la scocca del drone, sarà possibile impostare una quota obiettivo che il veivolo, controllato dal microcontrollore, punterà a raggiungere. Sempre attraverso messaggi MQTT sarà possibile ricevere informazioni sullo stato del drone o forzare un atterraggio graduale mediato dal MCU.

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
E' possibile inviare al drone tre diversi tipi di messaggio:
1. 'S' : comanda al drone di atterrare.
2. 'I' : chiede al drone di fornire informazioni circa il suo stato attuale.
3. 't:trg:esp' : imposta una quota obiettivo *trg* (intero, espresso in cm) con un margine di errore *eps* (intero, espresso in cm).

### Schema:
![alt](https://github.com/MatteoOnger/Drone/blob/main/Documents/Drone_schema.png)

### Note:
1. I quattro motori arrivano a richiedere fino a 2 Ampere per generare una spinta sufficiente al decollo, è dunque necessario verificare che non vi siano limitazioni sulla corrente massima di scarica della batteria utilizzata per alimentarli.
2. Le eliche adiacenti ruotano in verso opposto una all'altra nei quadrirotori. Anzichè gestire e collegare con cavi e pin differenti le coppie di motori che debbono muoversi in verso opposto, si è preferito trattare  ugualmente tutti gli attuatori sia a livello software sia a livello hardware fino ai ponti H, come se tutti dovessero lavorare nella stessa direzione, per poi invertire il collegamento  motore - ponte H. Questo riduce la complessità del circuito finale e del codice necessario.
4. E' necessario personalizzare il codice sorgente inserendo i dati della rete WiFi e del MQTT broker a cui si intende collegarsi e adattando alcuni parametri alla specifica configurazione.
5. Per migliorare il controllo del veivolo sarebbe opportuno aggiungere altri sensori (come accelerometri, giroscopi, ...), ma nel caso specifico si va incontro a problemi di spazio e peso, oltre al limite dato dal ridotto numero di pin disponibili.
