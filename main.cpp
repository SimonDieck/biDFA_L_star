#include "main.h"
#include "input_parser.h"
#include "experiments.h"
#include "models.h"

#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

void buildRunName(std::string &cleanName, std::string &fName){
    cleanName = std::string(fName, 0, fName.size()-4);
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "_%d-%m-%Y_%H--%M");
    std::string dateEx = oss.str();
    cleanName += dateEx;
}

int main(){

    std::string experimentFile = "simple_ws2_5r.txt";
    std::string cleanFileName;
    buildRunName(cleanFileName, experimentFile);
    std::string pathToExperiment = ".\\experiments\\";
    std::string pathToResults = ".\\results\\";

    int nExps = countExperimentsinFile((pathToExperiment+experimentFile));

    std::vector<BiDFA> teachers(nExps);
    
    std::vector<Experiment> experiments = experimentsFromFile((pathToExperiment+experimentFile), teachers);
    if(experiments.empty()){
        throw std::runtime_error("Error: No experiments could be read from file.");
    }
    writeExperimentFiles(cleanFileName, pathToExperiment, experiments);

    for(auto it = experiments.begin(); it != experiments.end(); ++it){
        (*it).run();
    }

    writeExperimentsResults(cleanFileName, pathToResults, experiments, true);

}
