
# CutePipes


## Integrantes
 - Isabela Muriel 
 -  Luisa María Vásquez


## Bibliotecas
- yaml-cpp ( https://yaml.org/ )
- pthread
## Ejecución
Para compilar CutePipes ejecute el comando:

    make

Posteriormente, el ejecutable deberá haberse creado en la carpeta bin, ejecute el comando, desde la raíz del repositorio:

    ./bin/sisctrl <archivo.yaml>
Recuerde que los comandos válidos son:

    { cmd: send , msg: <cadena> }
    { cmd: info , msg: "" }
    { cmd: info , msg: <automata> }
    { cmd: stop , msg: "" }


¡Que lo disfrute! :) <3
