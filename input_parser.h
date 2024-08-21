#ifndef INPUT_PARSE_L_STAR
#define INPUT_PARSE_L_STAR

#include "models.h"
#include "experiments.h"

#include <vector>
#include <iostream>
#include <map>
#include <string>

const static std::map<std::string, int> parameterID{
        {"AlphabetSize", 0}, {"EquivalenceMethod", 1}, {"W", 2}, {"EarlyStop", 3}, {"RandomSeed", 4}, 
        {"PreferLeft", 5}, {"WarmStart", 6}, {"ResetRemember", 7}};

const static std::map<std::string, int> metaDataID{
        {"ExpID", 0}, {"version", 1}, {"teacher", 2}, {"teacherType", 3}, {"repeats", 4}};

void parseBiDFA(std::string pathToFile, BiDFA &teacher);

void dfaToBiDFA(std::string pathToDFAFile, std::string pathTobiDFAFile);

void writeExperimentFile(std::string pathToOutput, Experiment& experiment);

void writeExperimentFiles(std::string batchName, std::string pathToOutput, 
                          std::vector<Experiment>& experiments);

int countExperimentsinFile(std::string pathToInput);

bool parseExperimentFile(std::ifstream& fs, std::vector<std::string>& metaData, 
                         std::vector<int>& parameters);

Experiment experimentFromFile(std::string pathToInput, BiDFA& pTeacher);

std::vector<Experiment> experimentsFromFile(std::string pathToInput, std::vector<BiDFA>& pTeachers);

void writeExperimentResults(std::ofstream& os, std::string pathToOutput, 
                            Experiment& experiment, bool dot=false);

void writeExperimentsResults(std::string batchName, std::string pathToOutput, 
                             std::vector<Experiment>& experiments, bool dot=false);

std::string buildDotNode(int acc, bool dir);

void biDFAToDot(std::string pathToInput, std::string pathToOutput);

#endif