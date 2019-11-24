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
void closePipesSisctrl(map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, map <string, vector<arista>> &graph);
void closeUnusedPipes(string &autom,string &name, map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error);
void testArrival(map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, map<string, map<string, int[2]>> &finals, string &name, string &autom);
void createPipe(int* p);
void process(string &s, map<string, map<string, int[2]>> &error,map<string, map<string, int[2]>> &finals,string &name, string &autom);


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
    vector<int> pids;
    for (unsigned int i = 0; i < doc.size(); i++) {
        string automata = doc[i]["automata"].as<string>();
        YAML::Node delta = doc[i]["delta"];
        for (unsigned j = 0; j < delta.size(); j++) {
            string node = delta[j]["node"].as<string>();
            if ((cpid = fork()) == 0) {
                closeUnusedPipes(automata,node,graph,finals,initial,error);
                testArrival(initial,error,finals,node,automata);
                exit(EXIT_SUCCESS);
            }
            pids.push_back(cpid);
        }
    }
    closePipesSisctrl(finals,initial,error, graph);
    for (auto const& mapa: initial){
        for (auto const& p: mapa.second) {
            string s = "Hola bb";
            write(p.second[1],s.c_str(), s.length());
            close(p.second[1]);
        }
    }
    for (auto const& mapa: error){
        for (auto const& p: mapa.second) {
            char buf;
            string s = "";
            while (read(p.second[0], &buf, 1) > 0)
                //string c(1,buf);
                s.append(&buf);
            cout << s << endl;
        }
    }
    int status;

    for (int id : pids) {
        waitpid(id, &status, 0);
    }
    cout << "Mis hijos murieron, me voy a suicidar" << endl;
    exit(EXIT_SUCCESS);
}

void closeUnusedPipes(string &autom,string &name, map <string, vector<arista>> &graph, map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error) {
    closeFinals(finals,name,autom);
    closeInitial(initial,name,autom);
    closeError(error, name, autom);
    closeEdges(graph,name,autom);
}

void closeFinals(map<string,map<string, int[2]>> &finals, string &name, string &autom){
    for (auto const& mapa: finals){
        for (auto const& p: mapa.second){
            if (p.first==name && mapa.first==autom){
                close(p.second[0]);
            }
            else {
                close(p.second[0]);
                close(p.second[1]);
            }

        }
    }
}

void closeInitial(map<string,map<string, int[2]>> &initial, string &name, string &autom){
    for (auto const& mapa: initial){
        for (auto const& p: mapa.second){
            if (p.first==name && mapa.first==autom){
                close(p.second[1]);
            }
            else {
                close(p.second[0]);
                close(p.second[1]);
            }
        }
    }
}

void closeError(map<string,map<string, int[2]>> &error, string &name, string &autom){
    for (auto const& mapa: error){
        for (auto const& p: mapa.second){
            if(p.first==name && mapa.first==autom){
                close(p.second[0]);
            }
            else{
                close(p.second[0]);
                close(p.second[1]);
            }
        }
    }
}

void closeEdges(map<string,vector<arista>> &graph, string &name,string &autom){
    for (auto const& mapa: graph){
        for (arista ar: mapa.second){
            if (mapa.first==autom){
                if (ar.nodeIn==name && ar.nodeOut!=name ){      
                    close(ar.pipefd[0]);
                    
                }
                else if (ar.nodeOut==name && ar.nodeIn!=name){
                    
                    close(ar.pipefd[1]);
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

void closePipesSisctrl(map<string, map<string, int[2]>> &finals, map<string, map<string, int[2]>> &initial, map<string, map<string, int[2]>> &error, map<string, vector<arista>> &graph){
     //Closing read in all error pipes
    for (auto const& mapa: error) {
        for (auto const& p: mapa.second){
            close(p.second[1]);
        }
    }
    //Closing write in all final pipes
    for (auto const& mapa: finals){
        for (auto const& p: mapa.second) {
            close(p.second[1]);
        }
    }
    //Closing all read in initial pipes
    for (auto const& mapa: initial){
        for (auto const& p: mapa.second) {
            close(p.second[0]);
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

void testArrival(map<string, map<string, int[2]>> &initial,map<string, map<string, int[2]>> &error,map<string, map<string, int[2]>> &finals, string &name, string &autom){
    
    for (auto const& mapa: initial){
        for (auto const& p: mapa.second) {
            if (p.first==name && mapa.first==autom){
                char buf;
                string s = "";
                while (read(p.second[0], &buf, 1) > 0)
                   //string c(1,buf);
                   s.append(&buf);
                process(s,error,finals,name,autom);
            }  
        }
    }
    
}

void process(string &s, map<string, map<string, int[2]>> &error,map<string, map<string, int[2]>> &finals,string &name, string &autom){
    s = s.append("procesada");
    for (auto const& mapa: error){
        for (auto const& p: mapa.second){
            if (p.first==name && mapa.first==autom){
                write(p.second[1],s.c_str(),s.length());
            }

        }
    }
}


void arista_tostring(arista edge) {
    cout << "Node In: " << edge.nodeIn << endl;
    cout << "Trans: " << edge.trans << endl;
    cout << "Node Out: " << edge.nodeOut << endl;
}