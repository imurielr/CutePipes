#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <bits/stdc++.h> 
 


int 
main(int argc, char *argv[])
{    printf("ehh amiga");
    int pipei[2]; // Pipe from parent to child for input
    int pipes[2];  //Pipe from child to child for processing
    int pipeo[2];  //Pipe from child to parent for error
    pid_t cpid;
    char buf;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (pipe(pipes) == -1 || pipe(pipei) == -1 || pipe(pipeo) == -1  ) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (cpid == 0) {         // Si soy el hijo
        close(pipei[1]);      //Cierro lado de escritura del input
        while(true){          

        char buf;
        close(pipeo[0]);       // Close read from output
        std::string chain ="";   // Mensaje
        while (read(pipei[0], &buf, 1) > 0){  //Leo caracter por caracter de input (porque no hay hilos)
            std::string s(1, buf);
            chain.append(s);
        }
        while (read(pipes[0], &buf, 1) > 0){ //Leo caracter por caracter de la tuberia a mi mismo (porque no hay hilos)
            std::string s(1, buf);
            chain.append(s);
        }
        
        char c = chain.at(0);   // Cojo el primer caracter
        if(c=='a'){
            chain =chain.substr(1);   // Lo elimino de la cadena 
            if(chain.length()==0){    // Si no queda nada la cadena se reconoce
            std::string r="R";
            write(pipeo[1],r.c_str(),1);  //Escribo en output 
            exit(EXIT_SUCCESS);
            }
            write(pipes[1],chain.c_str(),chain.length()-1); //Sino, solo lo mando otra vez a la tuberia de si mismo
        }else{
            std::string nr="NR";           //Si no es a, no pasa y no es reconocido
            write(pipeo[1],nr.c_str(),2);
            exit(EXIT_SUCCESS);
        }
        }

    } else {            
        close(pipei[0]);      //Cierro lectura de tuberia
        close(pipeo[1]);     // Cierro escritura del output
        close(pipes[0]);     // Cierro Tuberia que es solo del hijo
        close(pipes[1]);
        write(pipei[1], argv[1], strlen(argv[1]));  // Escribo cadena en input
        while(true){      // Espero respuesta
        char buf;
        std::string chain="";
        while (read(pipeo[0], &buf, 1) > 0){
            std::string s(1, buf);
            chain.append(s);
        }
        if(chain.length()>0){   
        printf(chain.c_str());  //La imprimo y salgo
        close(pipei[1]);    
        break;
        }
        }      /* Reader will see EOF */
        wait(NULL);                /* Wait for child */
        exit(EXIT_SUCCESS);
    }
}





    

