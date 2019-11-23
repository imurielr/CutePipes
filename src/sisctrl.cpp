#include <sys/types.h>
       #include <sys/wait.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <string.h>  
       #include <iostream>
        #include <vector>
        #include <yaml-cpp/yaml.h>


   using namespace std;
       int
       main(int argc, char *argv[])
       {    
           YAML::Node doc;
           try {
                doc = YAML::LoadFile("datos.yaml");
           } catch (exception& e) {
               cout << "El error es " << e.what() << endl;
           }
            
           cout<<doc[0]["invoice"].as<std::string>() <<endl;;
            return 0;

       }