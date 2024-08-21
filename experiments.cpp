#include "experiments.h"

#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>


Experiment::Experiment(std::string name, std::string pathToTeacher, std::string teachType, Model& pTeacher,
                       std::vector<int> &params) : algorithm(LStar(params[0], pTeacher))
{

    expIdentifier = name;
    this->pathToTeacher = pathToTeacher;
    this->teacherType = teachType;

    //Set unknown parameters to default value
    for(int i=0; i < parameterNames.size(); i++){
        if(params[i] == -1){
            params[i] = defaultParameter[i];
        }
    }
    //randomise random seed if non is given
    if(params[4] == -1){
        params[4] = std::chrono::system_clock::now().time_since_epoch().count();
        params[4] = params[4] % 10000;
    }
    
    algorithm.setParameters(params[1], params[2], params[3], params[4], params[5], params[6], params[7]);

    this->parameters = std::vector<int>(params.begin(), params.end());
}

void Experiment::run(){

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    std::string dateEx = oss.str();

    dateOfExecution = dateEx;

    startTime = std::chrono::steady_clock::now();
    algorithm.fit();
    endTime = std::chrono::steady_clock::now();

}

void Experiment::printSetup(std::ostream &out){

    out << "{\n";
    out << "\"ExpID\": \"" << expIdentifier << "\", \n";
    out << "\"version\": \"" << version << "\", \n";
    out << "\"teacher\": \"" << pathToTeacher << "\", \n";
    out << "\"teacherType\": \"" << teacherType << "\", \n";
    for(int i=0; i < parameterNames.size()-1; i++){
        out << "\"" << parameterNames[i] << "\": \"" << parameters[i] << "\", \n";
    }
    int last = parameterNames.size()-1;
    out << "\"" << parameterNames[last] << "\": \"" << parameters[last] << "\" \n";
    out << "}";

}

void Experiment::printResult(std::ostream &out){

    BiDFA result;

    algorithm.getFittedModel(result);

    out << "{\n";
    out << "\"ExpID\": \"" << expIdentifier << "\", \n";
    out << "\"Date of execution\": \"" << dateOfExecution << "\", \n";
    out << "\"Execution time\": \"" 
        << std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count() << "s\", \n";
    out << "\"Model size\": " << result.getSize() << ", \n";
    out << "\"Early stop\": ";
    if(result.getSize() == parameters[3]){
        out << "\"True\" \n";
    }else{
        out << "\"False\" \n";
    }
    out << "}";

}

void Experiment::printResultModel(std::ostream &out){

    BiDFA result;

    algorithm.getFittedModel(result);

    result.print(out);
    
}

std::string Experiment::getName(){

    return expIdentifier;

}
