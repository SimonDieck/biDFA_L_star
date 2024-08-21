#include "models.h"

#include <iostream>
#include <utility>

int Trace::nextChar(bool direction, bool ignoreCentre){
    if(direction){
        currentPosLeft++;
        if((currentPosLeft >= centre && !ignoreCentre) ||
            currentPosLeft >= length){
            return -1;
        }
        return characters[currentPosLeft];
    }else{
        currentPosRight++;
        if((length -currentPosRight -1 < centre && !ignoreCentre) ||
            currentPosRight >= length){
            return -1;
        }
        return characters[length-currentPosRight-1];
    }
}

void Trace::resetRead(){
    currentPosLeft = -1;
    currentPosRight = -1;
}

void Trace::vectorise(std::vector<int> &vTrace){
    vTrace = std::vector<int>(characters.begin(), characters.end());
}

Trace Trace::extend(Trace &interfix, bool inplace, int pLeft, int pRight){

    if(pLeft + pRight > interfix.length){
        throw std::runtime_error("Negative length interfix was given as extension.");
        return *this;
    }

    std::vector<int> vInterfix;
    interfix.vectorise(vInterfix);
    if(inplace){
        auto it = characters.begin();
        characters.insert(it+centre, vInterfix.begin()+pLeft, vInterfix.end()-pRight);
        centre = centre + interfix.centre + pLeft;
        length = length + interfix.length - (pLeft + pRight);
        return *this;
    }else{
        std::vector<int> newCharas = std::vector<int>(characters.begin(), characters.end());
        auto it = newCharas.begin();
        newCharas.insert(it+centre, vInterfix.begin()+pLeft, vInterfix.end()-pRight);
        int newCentre = centre + interfix.centre + pLeft;
        return Trace(accepted, newCharas, newCentre);
    }
    
}

bool Trace::equalUpTo(Trace &otherTrace, int pLeft, int pRight, bool ignoreCentre){

    //More positions than available need to be compared
    if(pLeft + pRight > otherTrace.length){
        return false;
    }

    std::vector<std::pair<int,bool>> sets = 
            {std::pair<int,bool>(pLeft, true), std::pair<int,bool>(pRight, false)};
            
    for(auto it = sets.begin(); it != sets.end(); ++it){
        for(int i = 1; i <= length; i++){
            if(i > (*it).first){
                break;
            }
            //TODO: remove tmp variables for testing
            int nCThis = nextChar((*it).second, ignoreCentre);
            int nCOther = otherTrace.nextChar((*it).second, ignoreCentre);
            if(nCThis != nCOther){
                resetRead();
                otherTrace.resetRead();
                return false;
            }
        }
        resetRead();
        otherTrace.resetRead();
    }

    return true;
}

Trace::Trace(){
    length = 0;
    characters = std::vector<int>();
    accepted = -1;
    centre = 0;
    resetRead();
}

Trace::Trace(ternary accepted, std::vector<int> &characters, int centre){
    length = characters.size();
    this->characters = std::vector<int>(characters.begin(), characters.end());
    this->accepted = accepted;
    if(centre == -1){
        this->centre = length;
    }else{
        this->centre = centre;
    }
    resetRead();
}

bool Model::memberQuery(Trace &trace){
    throw std::runtime_error("Undefined membership query was called.");
    return false;
}

bool Model::equivQuery(Model &model, Trace &counterexample){
    throw std::runtime_error("Undefined equivalence query was called.");
    return false;
}

int Node::followEdge(int c){
    return transitions[c];
}

Node::Node(ternary accepting, std::vector<int> &transitions, bool orientation){
    this->accepting = accepting;
    this->transitions = std::vector<int>(transitions.begin(),transitions.end());
    this->orientation = orientation;
}

BiDFA::BiDFA(){
    nNodes = 0;
    nCharacters = 0;
    Model::mType = "BiDFA";
}

BiDFA::BiDFA(int nCharacters, int nNodes, std::vector<Node> &nodes) 
        : nodes(std::vector<Node>(nodes.begin(), nodes.end())){
    this->nCharacters = nCharacters;
    this->nNodes = nNodes;
    Model::mType = "BiDFA";
}

int BiDFA::getSize(){
    return nNodes;
}

ternary BiDFA::parseTrace(Trace &trace){
    int state = followTrace(trace);
    return nodes[state].accepting;
}

int BiDFA::followEdge(int node, int cChar){
    return nodes[node].followEdge(cChar);
}

int BiDFA::followTrace(Trace &trace){

    int cState = 0;
    for(int i = 0; i < trace.length; i++){
        bool orientation = nodes[cState].orientation;
        int cChar = trace.nextChar(orientation, true);
        if(cChar == -1){
            std::cout << "Warning: Undefined character read.";
            return -1;
        }
        cState = nodes[cState].followEdge(cChar);
        if(cState == -1){
            std::cout << "Warning: Undefined state encountered.";
            return -1;
        }
    }
    trace.resetRead();
    return cState;
}

bool BiDFA::getOrientation(int node){
    return nodes[node].orientation;
}

bool BiDFA::memberQuery(Trace &trace){
    ternary results = parseTrace(trace);
    if(results == 1){
        return true;
    }else{
        return false;
    }
}

bool BiDFA::equivQuery(Model &model, Trace &counterexample){
    //TODO
    return false;
}

void BiDFA::print(std::ostream &out){

    out << nNodes << " " << nCharacters << "\n";

    for(int i=0; i < nNodes; i++){
        if(nodes[i].orientation){
            out << i << " " << nodes[i].accepting << " 1 \n";
        }else{
            out << i << " " << nodes[i].accepting << " 0 \n";
        }
        for(int c=0; c < nCharacters; c++){
            out << i << " " << c << " " << nodes[i].followEdge(c) << "\n";
        }
    }
}