#include "input_parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <random>
#include <chrono>

void parseBiDFA(std::string pathToFile, BiDFA &teacher){

    std::ifstream fs (pathToFile);

    std::string line;
    int nNodes;
    int nChars;

    bool warningFlag = false;

    if(std::getline(fs, line)){
        std::istringstream ss(line);
        ss >> nNodes >> nChars;
    }
    int node;
    int acc;
    int dir;
    int c;
    int to;
    std::vector<bool> directions(nNodes, true);
    std::vector<int> accepting(nNodes, -1);
    std::vector<std::vector<int>> transitions(nNodes, std::vector<int>(nChars, -1));
    for(int i=0; i < nNodes; i++){
        if(std::getline(fs, line)){
            std::istringstream ss(line);
            ss >> node >> acc;
            if(!ss){
                if(!warningFlag){
                    std::cout << "Warning: Gave DFA file to BiDFA parser. \n" 
                              << "All directions will be assumed as left \n";
                    warningFlag=true;
                }
                dir = 1;
            }else{
                ss >> dir;
            }
            directions[node] = dir;
            accepting[node] = acc;
            for(int c=0; c < nChars; c++){
            if(std::getline(fs, line)){
                std::istringstream ss(line);
                ss >> node >> c >> to;
                transitions[node][c] = to;
                }else{
                    throw std::runtime_error("Incomplete model description given to parser.");
                }
            }
        }else{
            throw std::runtime_error("Incomplete model description given to parser.");
        }
    }

    std::vector<Node> nodes;
    for(int i=0; i < nNodes; i++){
        nodes.push_back(Node(accepting[i], transitions[i], directions[i]));
    }

    teacher = BiDFA(nChars, nNodes, nodes);

    fs.close();

}

void dfaToBiDFA(std::string pathToDFAFile, std::string pathTobiDFAFile){
    //TODO
}

void writeExperimentFile(std::string pathToOutput, Experiment &experiment){

    std::string fpath = pathToOutput + experiment.getName() + "_full_setup.txt";

    std::ofstream os(fpath);

    experiment.printSetup(os);

    os.close();

}

void writeExperimentFiles(std::string batchName, std::string pathToOutput, 
                          std::vector<Experiment> &experiments){

    std::string batchPtO = pathToOutput + batchName + "_full_setup.json";
    std::ofstream os(batchPtO);
    os << "{ \n\"" << batchName <<"\":\n[\n"; 
    for(auto it = experiments.begin(); it != experiments.end(); ++it){
        (*it).printSetup(os);
        if(it+1 != experiments.end()){
            os << ",\n";
        }
    }
    os << "\n]\n}";
    
}

int countExperimentsinFile(std::string pathToInput){
    int count = 0;
    std::string line;
    std::ifstream fs(pathToInput);
    while(std::getline(fs, line, '}')){
        count ++;
    }
    fs.close();
    fs = std::ifstream(pathToInput);
    if(std::getline(fs, line, ']')){
        return count-1;
    }else{
        return 1;
    }
    
}

bool parseExperimentFile(std::ifstream& fs, std::vector<std::string> &metaData, 
                         std::vector<int> &parameters){

    std::string exp;
    std::string line;
    std::string pName;
    std::string colon;
    std::string mData;
    int param;

    if(!std::getline(fs, exp, '}')){
        return false;
    }

    std::istringstream is(exp);

    if(!std::getline(is, line)){
        return false;
    }

    while(std::getline(is, line, ',')){

        std::regex reg("[\":]+");

        std::sregex_token_iterator iter(line.begin(), line.end(), reg, -1);
        std::sregex_token_iterator end;
 
        std::vector<std::string> tokens(iter, end);

        if(tokens.size() >= 3){
            pName = tokens[1];
            if(auto search = metaDataID.find(pName); search != metaDataID.end()){
                mData = tokens[2];
                metaData[(search->second)] = mData;
            }else if(auto search = parameterID.find(pName); search != parameterID.end()){
                param = std::stoi(tokens[2]);
                parameters[(search->second)] = param;
            }else{
                std::cout << "Warning: Unknown parameter " << pName << " given. \n";
            }
        }else{
            break;
        }
    }

    if(metaData[metaDataID.find("ExpID")->second].empty()){
        throw std::runtime_error("Experiment file contained no name for the experiment.");
    }
    if(metaData[metaDataID.find("teacher")->second].empty()){
        throw std::runtime_error("Experiment file contained no path to a teacher.");
    }
    if(metaData[metaDataID.find("teacherType")->second].empty()){
        throw std::runtime_error("Type of teacher was not given.");
    }
    if(metaData[metaDataID.find("repeats")->second].empty()){
        metaData[4] = "1";
    }
    if(parameters[parameterID.find("AlphabetSize")->second] == -1){
        throw std::runtime_error("No alphabet size was given in the experiment.");
    }

    return true;

}

//deprecated
Experiment experimentFromFile(std::string pathToInput, BiDFA& pTeacher){

    std::vector<std::string> metaData(metaDataID.size(), "");
    std::vector<int> parameters(parameterID.size(), -1);

    std::ifstream fs(pathToInput);

    bool success = parseExperimentFile(fs, metaData, parameters);
    if(!success){
        throw std::runtime_error("Empty experiment file given to parser.");
    }

    fs.close();

    parseBiDFA(metaData[metaDataID.find("teacher")->second], pTeacher);

    return Experiment(metaData[metaDataID.find("ExpID")->second], 
                      metaData[metaDataID.find("teacher")->second], 
                      metaData[metaDataID.find("teacherType")->second], pTeacher, parameters);

}

std::vector<Experiment> experimentsFromFile(std::string pathToInput, std::vector<BiDFA> &pTeachers){

    std::ifstream fs(pathToInput);
    bool success = true;
    std::string line;
    std::vector<Experiment> experiments;
    int cParsed = 0;
    std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(0, 99999);

    if(!std::getline(fs, line, '[')){
        fs.close();
        std::ifstream fs(pathToInput);
    }else{
        std::getline(fs, line);
    }
    while(success){
        std::vector<std::string> metaData(metaDataID.size(), "");
        std::vector<int> parameters(parameterID.size(), -1);

        success = parseExperimentFile(fs, metaData, parameters);
        if(parameters[parameterID.find("RandomSeed")->second] != -1){
            gen = std::default_random_engine(parameters[parameterID.find("RandomSeed")->second]);
        }
        if(!success){
            break;
        }
        int repeats = std::stoi(metaData[metaDataID.find("repeats")->second]);
        for(int i = 1; i <= repeats; i++){
            std::string name = metaData[metaDataID.find("ExpID")->second];
            if(repeats > 1){
                name = name + "_run_" + std::to_string(i);
                parameters[parameterID.find("RandomSeed")->second] = dist(gen);
            }
            parseBiDFA(metaData[metaDataID.find("teacher")->second], pTeachers[cParsed]);
            Experiment tmpExp(name, metaData[metaDataID.find("teacher")->second], 
                              metaData[metaDataID.find("teacherType")->second], 
                              pTeachers[cParsed], parameters);
            experiments.push_back(tmpExp);
        }
        cParsed++;
        if(!std::getline(fs, line) || cParsed >= pTeachers.size()){
            break;
        }
    }
    fs.close();
    return experiments;
}

void writeExperimentResults(std::ofstream& os, std::string pathToOutput, 
                            Experiment &experiment, bool dot){

    std::string name = experiment.getName();

    std::filesystem::create_directory(pathToOutput+"result_models\\");

    std::string modelPath = pathToOutput + "result_models\\" + name + "_result_model.txt";

    experiment.printResult(os);

    std::ofstream os2(modelPath);
    experiment.printResultModel(os2);
    os2.close();

    if(dot){
        std::filesystem::create_directory(pathToOutput+"result_models\\results_dot\\");
        std::string dotPath = pathToOutput + "result_models\\results_dot\\" + name + "_result_model.dot";
        biDFAToDot(modelPath, dotPath);
    }
    
}

void writeExperimentsResults(std::string batchName, std::string pathToOutput, 
                             std::vector<Experiment> &experiments, bool dot){
    std::string batchPtO = pathToOutput + batchName + "\\";
    std::filesystem::create_directory(batchPtO);
    std::string dataPath = batchPtO + batchName + "_result.json";
    std::ofstream os(dataPath);
    os << "{\n\"" << batchName << "\":[\n";
    for(auto it=experiments.begin(); it != experiments.end(); ++it){
        writeExperimentResults(os, batchPtO, *it, dot);
        if(it+1 != experiments.end()){
            os << ",\n";
        }
    }
    os << "\n]\n}";
    os.close();

}

std::string buildDotNode(int acc, bool dir){

    std::string modifier = "[";
    if(dir){
        modifier += "color=red, ";
    }else{
        modifier += "color=blue, ";
    }
    modifier += "style=striped, ";
    if(acc == 1){
        modifier += "shape=doublecircle]";
    }else if(acc == 0){
        modifier += "shape=Mcircle]";
    }

    return modifier;
}

void biDFAToDot(std::string pathToInput, std::string pathToOutput){

    std::ifstream is(pathToInput);
    std::ofstream os(pathToOutput);

    os << "digraph{\n";

    std::string line;
    int nNodes;
    int alphabetSize;
    if(std::getline(is, line)){
        std::istringstream iss(line);
        iss >> nNodes >> alphabetSize;
    }
    std::vector<std::vector<int>> adjMat(nNodes, std::vector<int>(alphabetSize, 0));
     
    while(std::getline(is, line)){
        int id;
        int acc;
        int dir;
        std::istringstream iss(line);
        iss >> id >> acc >> dir;
        os << std::to_string(id) << " " << buildDotNode(acc, dir) << "\n";
        for(int i=0; i < alphabetSize; i++){
            if(std::getline(is, line)){
                int from;
                int c;
                int to;
                std::istringstream iss2(line);
                iss2 >> from >> c >> to;
                adjMat[from][c] = to;
            }else {
                throw std::runtime_error("On building .dot the result biDFA is only partially defined.");
            }
        }
    }

    is.close();
    for(int i=0; i < nNodes; i++){
        for(int c=0; c < alphabetSize; c++){
            os << std::to_string(i) << "->" << std::to_string(adjMat[i][c]); 
            os << " [label=" << std::to_string(c) << "]\n";
        }
    }
    os << "}";
    os.close();
    
}
