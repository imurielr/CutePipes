#include <sys/types.h>
       #include <sys/wait.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <string.h>  
       #include <iostream>

       using namespace std;

       int
       main(int argc, char *argv[])
       {
           int pipefd[2];
           int pipef[2];
           int pipes[2];
           pid_t cpid;
           char buf;
           
           if (pipe(pipefd) == -1 || pipe(pipef)==-1 || pipe(pipes)==-1) {
               perror("pipe");
               exit(EXIT_FAILURE);
           }
           
           cpid = fork();
           if (cpid == -1) {
               perror("fork");
               exit(EXIT_FAILURE);
           }  
              
           if (cpid == 0) { 
               close(pipefd[1]);   //Child process
               close(pipef[0]);
               std::string chain ="";
               char buf;
               while (read(pipefd[0], &buf, 1) > 0){  //Leo caracter por caracter de input (porque no hay hilos)
                std::string s(1, buf);
                chain.append(s);
               }
           
                printf("Hello");
                if(chain.length()==1){
                    if(chain.find('a') == 0){
                     close(pipefd[0]);
                     std::string s ="R";
                     write(pipef[1],s.c_str(),1);
                       close(pipef[1]);
                       
                     _exit(EXIT_SUCCESS);
                    }else{
                        close(pipefd[0]);
                     std::string s ="D";
                     write(pipef[1],s.c_str(),1);
                       close(pipef[1]);
                     _exit(EXIT_SUCCESS);
                    }
                    
                }else{
                    
                    char c = chain.at(0);
                    
                    cout<<c;
                    _exit(EXIT_SUCCESS);
                    //const char* cha = chain.substr(1).c_str();
                    char* cha ="ju";
                    if(c=='a'){
                        
                        //write(pipes[1],&cha,3);
                        
                    }else{
                        close(pipefd[0]);
                     std::string s ="D";
                     write(pipef[1],s.c_str(),1);
                       close(pipef[1]);
                     _exit(EXIT_SUCCESS);
                    }
                    
                }
                chain ="";
                
                while (read(pipes[0], &buf, 1) > 0){  //Leo caracter por caracter de input (porque no hay hilos)
                    std::string s(1, buf);
                    chain.append(s);
                }
                printf(chain.c_str());
           
           }
           else {  //Parent process
               close(pipefd[0]);
               close(pipef[1]);
               close(pipes[0]);
               close(pipes[1]);
               char buf;
               write(pipefd[1],argv[1] , strlen(argv[1]));
               close(pipefd[1]);
               //read(pipef[0], &buf, 1) ;
                   cout << "\nExiting program..."<<buf;
                   close(pipef[0]);
                   int status;
                   waitpid(cpid, &status, 0);
                   exit(EXIT_SUCCESS);
               
           }  
       }