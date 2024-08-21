#ifndef EXPERIMENTS_BIDFA_L_STAR
#define EXPERIMENTS_BIDFA_L_STAR

#include "l_star.h"
#include "models.h"

#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <vector>

const static std::vector<std::string> parameterNames{
        "AlphabetSize", "EquivalenceMethod", "W", "EarlyStop", "RandomSeed", "PreferLeft", "WarmStart",
        "ResetRemember"};
const static std::vector<int> defaultParameter{
        -1 , 1, 3, 100, -1, 0, 0, 0};

class Experiment{

    std::string version = "0.1";
    std::string expIdentifier;

    std::string pathToTeacher;
    std::string teacherType;

    std::vector<int> parameters;

    LStar algorithm;

    std::string dateOfExecution = "not_run";

    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;

    public:

        Experiment(std::string name, std::string pathToTeacher, std::string teachType, Model& pTeacher, 
                   std::vector<int>& params);
        
        void run();

        void printSetup(std::ostream& out);
    
        void printResult(std::ostream& out);

        void printResultModel(std::ostream& out);

        std::string getName();

};

#endif