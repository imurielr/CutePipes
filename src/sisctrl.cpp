#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <thread>
#include <semaphore.h>

using namespace std;

struct arista {
    string nodeIn;
    string nodeOut;
    string trans;
    int pipefd[2];
};

void arista_tostring(arista edge);

void closeFinals(map<string,map<string, int[2]>> &finals, string &name, string &autom,bool &t);
void closeInitial(map<string,map<string, int[2]>> &initial, string &name, string &autom ,bool &t);
void closeError(map<string,map<string, int[2]>> &error, string &name, string &autom ,bool &t);
void closeEdges(map<string,vector<arista>> &graph, string &name,string &autom ,bool &t);
void threadIteration(string autom, string name ,int pipe);

void createChild(YAML::Node& doc,map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error);
void closePipesSisctrl(map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, map <string, vector<arista>> &graph, bool &t);
void closeUnusedPipes(string &autom,string &name, map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, bool &t);
void createThreads(string &autom,string &name, map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error);
void inOutOperations(map<string,map<string,int>> pids, map<string,map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, map<string, vector<arista>> &graph);
void createPipe(int* p);

sem_t mutex1;
map <string, vector<arista>> grafo;


int main(int argc, char *argv[]) {
    YAML::Node doc;

    if (argc != 2) {
        cerr << "Usage: ./sisctrl <file.yaml>" << endl;
        exit(EXIT_FAILURE);
    }


    try {
        doc = YAML::LoadFile(argv[1]);
    }
    catch (exception &e) {
        cout << "El error es " << e.what() << endl;
    }

    arista edge;
    map<string, map<string, int[2]>> error, finals, initial;
    // vector<string> states;
    for (unsigned int i = 0; i < doc.size(); i++) { // Ciclo por cada automata
        YAML::Node delta = doc[i]["delta"];
        vector<arista> aristas;
        string nomAutomata = doc[i]["automata"].as<string>();
        createPipe(initial[nomAutomata][doc[i]["start"].as<string>()]);

        for (int f = 0; f < doc[i]["final"].size(); f++) {            
            createPipe(finals[nomAutomata][doc[i]["final"][f].as<string>()]);
        }


        for (unsigned int j = 0; j < delta.size(); j++) {
            YAML::Node trans = delta[j]["trans"];

            createPipe(error[nomAutomata][delta[j]["node"].as<string>()]);

            for (unsigned int k = 0; k < trans.size(); k++) {
                edge.nodeIn = delta[j]["node"].as<string>();
                edge.nodeOut = trans[k]["next"].as<string>();
                edge.trans = trans[k]["in"].as<string>();
                createPipe(edge.pipefd);
                aristas.push_back(edge);
                grafo[nomAutomata] = aristas;
            }
        }
    }
    createChild(doc,grafo,finals,initial,error);

    return 0;
}

void createPipe(int* p){
      if(pipe(p)==-1){
          //Error creando tuberia
          exit(EXIT_FAILURE);
      }
}

void createChild(YAML::Node& doc,map<string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error) {
    int cpid;
    map<string,map<string,int>> pids;
    for (unsigned int i = 0; i < doc.size(); i++) {
        string automata = doc[i]["automata"].as<string>();
        YAML::Node delta = doc[i]["delta"];
        for (unsigned j = 0; j < delta.size(); j++) {
            string node = delta[j]["node"].as<string>();
            if ((cpid = fork()) == 0) {
                bool ps =false;
                closeUnusedPipes(automata,node,graph,finals,initial,error,ps); // Falso - Cerrar no usadas, True - Cerrar usadas
                
                createThreads(automata,node,graph, finals,initial, error);
                //
                //
                //
                cout << "Pos me mato" << endl;
                ps=true;
                closeUnusedPipes(automata,node,graph,finals,initial,error,ps);
                
                exit(EXIT_SUCCESS);
            }
            pids[automata][node]=cpid;
        }
    }
    bool ss=false;
    closePipesSisctrl(finals,initial,error, graph,ss);
    inOutOperations(pids,finals,initial,error,graph);

    
}

void closeUnusedPipes(string &autom,string &name, map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, bool &t) {
    closeFinals(finals,name,autom,t);
    closeInitial(initial,name,autom,t);
    closeError(error, name, autom,t);
    closeEdges(graph,name,autom,t);
}

void closeFinals(map<string,map<string, int[2]>> &finals, string &name, string &autom ,bool &t){
    for (auto const& mapa: finals){
        for (auto const& p: mapa.second){
            if (p.first==name && mapa.first==autom ){
                if(!t){
                    close(p.second[0]);
                }else{
                    close(p.second[1]);
                }
            }
            else {
                close(p.second[0]);
                close(p.second[1]);
            }

        }
    }
}

void closeInitial(map<string,map<string, int[2]>> &initial, string &name, string &autom,bool &t){
    for (auto const& mapa: initial){
        for (auto const& p: mapa.second){
            if (p.first==name && mapa.first==autom){
                if(!t){
                    close(p.second[1]);
                }else{
                    close(p.second[0]);
                }
            }
            else {
                close(p.second[0]);
                close(p.second[1]);
            }
        }
    }
}

void closeError(map<string,map<string, int[2]>> &error, string &name, string &autom ,bool &t){
    for (auto const& mapa: error){
        for (auto const& p: mapa.second){
            if(p.first==name && mapa.first==autom){
                if(!t){
                    close(p.second[0]);
                }else{
                    close(p.second[1]);
                }
            }
            else{
                close(p.second[0]);
                close(p.second[1]);
            }
        }
    }
}

void closeEdges(map<string,vector<arista>> &graph, string &name,string &autom ,bool &t){
    for (auto const& mapa: graph){
        for (arista ar: mapa.second){
            if (mapa.first==autom){
                if (ar.nodeIn==name && ar.nodeOut!=name ){  
                    if(!t){    
                        close(ar.pipefd[0]);
                    }else{
                        close(ar.pipefd[1]);
                    }
                    
                }
                else if (ar.nodeOut==name && ar.nodeIn!=name){
                    if(!t){
                        close(ar.pipefd[1]);
                    }else{
                        close(ar.pipefd[0]);
                    }
                }
                else if (ar.nodeOut!=name && ar.nodeIn!=name){
                    
                    close(ar.pipefd[0]);
                    close(ar.pipefd[1]);
                }
            }
            else {
                close(ar.pipefd[0]);
                close(ar.pipefd[1]);
            }
        }
    }
}

void closePipesSisctrl(map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, map<string, vector<arista>> &graph, bool &t){
     //Closing writw in all error pipes
    for (auto const& mapa: error) {
        for (auto const& p: mapa.second){
            if(!t){
                close(p.second[1]);
            }else{
                close(p.second[0]);
            }
        }
    }
    //Closing write in all final pipes
    for (auto const& mapa: finals){
        for (auto const& p: mapa.second) {
            if(!t){
                close(p.second[1]);
            }else{
                close(p.second[0]);
            }
        }
    }
    //Closing all read in initial pipes
    for (auto const& mapa: initial){
        for (auto const& p: mapa.second) {
            if(!t){
                close(p.second[0]);
            }else{
                close(p.second[1]);
            }
        }
    }
    //Closing all read in graph pipes
    for (auto const& mapa : graph) {
        for (arista ar : mapa.second) {
            close(ar.pipefd[0]);
            close(ar.pipefd[1]);
        }
    }
}

void createThreads(string &autom,string &name, map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error){

    vector<thread> threads;
    
 
    if(initial[autom].count(name)>0){
        int pipe =initial[autom][name][0];
        thread thinit(threadIteration,autom, name,pipe);
        threads.push_back(move(thinit));
    } 
    for(arista ar : graph[autom]){
        if(ar.nodeOut==name){
            thread thar(threadIteration,autom, name,ar.pipefd[0]);
            threads.push_back(move(thar));
        }
    } 
    for(int i=0;i<threads.size();i++){
        threads[i].join();
    } 
}

void threadIteration(string autom, string name,int pipe){

    
}

void errRead(int pipe){

}

void finalRead(int pipe){

}



void inOutOperations(map<string,map<string,int>> pids, map<string,map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, map<string, vector<arista>> &graph){
     vector<thread> threads;
     for(auto const& map : error){
         for(auto const& p: map.second){
            thread readth(errRead,p.second[0]);
         }
     }

     for(auto const& map : finals){
         for(auto const& p: map.second){
            thread readth(finalRead,p.second[0]);
         }
     }


    for(int i=0;i<threads.size();i++){
        threads[i].join();
    } 
    
    int status;
    for(auto const& map : pids){
        for (auto const& p: map.second) {
            
            waitpid(p.second, &status, 0);
        }
    }
   
    bool ss=true;
    closePipesSisctrl(finals,initial,error, graph,ss);
    exit(EXIT_SUCCESS);

}

void userThread(){
    string linea, cmd, msg;
    getline(cin,linea);
    
    YAML::Node doc = YAML::Load(linea);
    
    cmd = doc["cmd"].as<string>();
    msg = doc["msg"].as<string>();
    cout << cmd << "  " << msg<< endl;
}


void arista_tostring(arista edge) {
    cout << "Node In: " << edge.nodeIn << endl;
    cout << "Trans: " << edge.trans << endl;
    cout << "Node Out: " << edge.nodeOut << endl;
}