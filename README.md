# Controllo Altitudine Drone

Autore: Matteo Onger 

Descrizione: Il progetto punta a recuperare il telaio e i motori di un drone già esistente ma danneggiato, sostituendone la circuiteria di controllo e programmandola     affinché, tramite messaggi MQTT, sia possibili controllare remotamente il quadrirotore. In particolare, sfruttando un sensore ad ultrasuoni posto sotto la                scocca del drone, sarà possibile impostare una quota obiettivo che il veivolo, controllato dal microcontrollore, punterà a raggiungere. Sempre attraverso                messaggi MQTT sarà possibile ricevere informazioni sullo stato del drone o forzare un atterraggio graduale mediato del MCU.