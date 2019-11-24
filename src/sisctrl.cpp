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

using namespace std;

struct arista {
    string nodeIn;
    string nodeOut;
    string trans;
    int pipefd[2];
};

void arista_tostring(arista edge);

void closeFinals(map<string,map<string, int[2]>> &finals, string &name, string &autom);
void closeInitial(map<string,map<string, int[2]>> &initial, string &name, string &autom);
void closeError(map<string,map<string, int[2]>> &error, string &name, string &autom);
void closeEdges(map<string,vector<arista>> &graph, string &name,string &autom);

void createChild(YAML::Node& doc,map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error);
void closePipesSisctrl(map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error);
void closeUnusedPipes(string &autom,string &name, map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error);

void createPipe(int* p){
      if(pipe(p)==-1){
          //Error creando tuberia
          exit(EXIT_FAILURE);
      }
}



map <string, vector<arista>> grafo;


int main(int argc, char *argv[]) {
    YAML::Node doc;


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

                // arista_tostring(grafo[doc[i]["automata"].as<string>()][k]);
                // cout << "Automata: " << nomAutomata << "   " << "Estado: " << states[nomAutomata][k] << endl;
            }
        }
    }
    createChild(doc,grafo,finals,initial,error);
    //     cout << "----------------------------" << endl;

    // for (auto const& x : states) {
    //     for (auto const& s : x.second) {
    //         cout << "Automata: " << x.first << "   " << endl;
    //     }
    // }

    // for (auto const& x : grafo) {
    //     for (unsigned int i = 0; i < x.second.size(); i++) {
    //         arista_tostring(grafo[x.first][i]);
    //     }
    // }

    return 0;
}

void createChild(YAML::Node& doc,map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error) {
    int cpid;
    vector<int> pids;
    for (unsigned int i = 0; i < doc.size(); i++) {
        string automata = doc[i]["automata"].as<string>();
        YAML::Node delta = doc[i]["delta"];
        for (unsigned j = 0; j < delta.size(); j++) {
            string node = delta[j]["node"].as<string>();
            if ((cpid = fork()) == 0) {
                closeUnusedPipes(automata,node,graph,finals,initial,error);
                exit(EXIT_SUCCESS);
            }
            pids.push_back(cpid);
        }
    }
    closePipesSisctrl(finals,initial,error);
    int status;
    for (int id : pids) {
        waitpid(id, &status, 0);
    }
    cout << "Mis hijos murieron, me voy a suicidar" << endl;
    exit(EXIT_SUCCESS);
}

void closeUnusedPipes(string &autom,string &name, map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error) {
    //closeFinals(finals,name,autom);
    //closeInitial(initial,name,autom);
    //closeError(error, name, autom);
    //closeEdges(graph,name,autom);
}

void closeFinals(map<string,map<string, int[2]>> &finals, string &name, string &autom){
    for(auto const& mapa: finals){
        for(auto const& p: mapa.second){
            if(p.first==name && mapa.first==autom){
                cout << name << " is final in " << autom << endl;
                close(p.second[0]);
            }else{
                close(p.second[0]);
                close(p.second[1]);
            }

        }
    }
}
void closeInitial(map<string,map<string, int[2]>> &initial, string &name, string &autom){
    for(auto const& mapa: initial){
        for(auto const& p: mapa.second){
            if(p.first==name && mapa.first==autom){
                cout << name << " is initial in " << autom << endl;
                close(p.second[1]);
            }else{
                close(p.second[0]);
                close(p.second[1]);
            }

        }
    }
}
void closeError(map<string,map<string, int[2]>> &error, string &name, string &autom){
    for(auto const& mapa: error){
        for(auto const& p: mapa.second){
            if(p.first==name && mapa.first==autom){
                cout << name << " has error pipe in " << autom << endl;
                close(p.second[0]);
            }else{
                close(p.second[0]);
                close(p.second[1]);
            }

        }
    }
}
void closeEdges(map<string,vector<arista>> &graph, string &name,string &autom){
    for(auto const& mapa: graph){
        for(arista ar: mapa.second){
            if(mapa.first==autom){
                if(ar.nodeIn==name && ar.nodeOut!=name ){
                    
                    close(ar.pipefd[0]);
                    cout<< " Closing read from " << ar.nodeIn <<" to "<< ar.nodeOut << " for " << name << " in " << autom<<endl;
                }else if(ar.nodeOut==name && ar.nodeIn!=name){
                    cout<< " Closing write from " << ar.nodeIn <<" to "<< ar.nodeOut << " for " << name << " in " << autom<<endl;
                    close(ar.pipefd[1]);
                }else if(ar.nodeOut!=name && ar.nodeIn!=name){
                    cout<< " Closing trasition from " << ar.nodeIn <<" to "<< ar.nodeOut << " for " << name << " in " << autom<<endl;
                    close(ar.pipefd[0]);
                    close(ar.pipefd[1]);
                }
            }else{
                cout<< " Closing trasition from " << ar.nodeIn <<" to "<< ar.nodeOut << " for " << name << " in " << autom<<endl;
                close(ar.pipefd[0]);
                close(ar.pipefd[1]);
            }
        }
    }
}

void closePipesSisctrl(map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error){
     //Closing read in all error pipes
    for(auto const& mapa: error){
        for(auto const& p: mapa.second){
            close(p.second[0]);
            cout<< " Closing read in " << p.first << " in " << mapa.first<<endl;
        }
    }
    //Closing write in all final pipes
    for(auto const& mapa: finals){
        for(auto const& p: mapa.second){
            close(p.second[1]);
            cout<< " Closing write in " << p.first << " in " << mapa.first<<endl;
        }
    }
    //Closing all read in initial pipes
    for(auto const& mapa: initial){
        for(auto const& p: mapa.second){
            close(p.second[0]);
            cout<< " Closing read in " << p.first << " in " << mapa.first<<endl;
        }
    }
}




void arista_tostring(arista edge) {
    cout << "Node In: " << edge.nodeIn << endl;
    cout << "Trans: " << edge.trans << endl;
    cout << "Node Out: " << edge.nodeOut << endl;
}