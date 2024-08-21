#include "models.h"
#include "l_star.h"

#include <iostream>
#include <cmath>


BiTree::BiTree() : left(nullptr), right(nullptr){

    id = -1;
}

BiTree::~BiTree(){
    if(left){delete left;}
    if(right){delete right;}
}

int BiTree::find(std::vector<bool> &bSeq, int currentPos){
    if(currentPos >= bSeq.size()){
        return id;
    }
    if(bSeq[currentPos] && left){
        return (*left).find(bSeq, currentPos+1);
    }else if(!bSeq[currentPos] && right){
        return (*right).find(bSeq, currentPos+1);
    }else{
        return -1;
    }
}

void BiTree::insert(std::vector<bool> &bSeq, int rid, int currentPos){
    if(currentPos >= bSeq.size()){
        id = rid;
        return;
    }
    if(bSeq[currentPos]){
        if(!left){
            BiTree * newNode = new BiTree();
            left = newNode;
        }
        (*left).insert(bSeq, rid, currentPos+1);
    }else{
        if(!right){
            BiTree * newNode = new BiTree();
            right = newNode;
        }
        (*right).insert(bSeq, rid, currentPos+1);
    }
}

void BiTree::getUniqueRows(std::vector<std::vector<int>> &rowBuckets){
    if(left){
        (*left).getUniqueRows(rowBuckets);
    }
    if(right){
        (*right).getUniqueRows(rowBuckets);
    }
    if(!left && !right){
        rowBuckets.push_back(std::vector<int>{id});
    }
}

void LStar::initialise(){

    nRowsS = 1;

    accessSeqs.push_back(Trace());

    if(firstRun){
        nColumns = 0;
        std::vector<bool> emptyObs;
        observationTable.push_back(emptyObs);
    }
    rowFinder.insert(observationTable[0], 0);
    std::vector<Trace> newExtensions;
    std::vector<std::vector<bool>> observations;
    bool newOri = determineDirection(nRowsS-1, newExtensions, observations);
    orientation.push_back(newOri);

    extdAccessSeqs.push_back(newExtensions);
    extendedTable.push_back(observations);

    if(firstRun){
        warmStartColumns(warmStart);
    }

}

bool LStar::remember(){
    if(firstRun){
            bestHypotheses = hypotheses;
            firstRun = false;
        }else if(hypotheses.getSize() <= bestHypotheses.getSize() 
                 || bestHypotheses.getSize() == prevEarlyStop){
            if(hypotheses.getSize() == bestHypotheses.getSize()){
                return false;
            }else{
                bestHypotheses = hypotheses;
            }
        }
        if(earlyStop == fullEarlyStop){
            return false;
        }else{
            prevEarlyStop = hypotheses.getSize();
            earlyStop = 2*earlyStop;
            if(earlyStop > fullEarlyStop){
                earlyStop = fullEarlyStop;
            }
        }
        return true;
}

void LStar::reset()
{

    std::vector<std::vector<bool>> newObsTable;
    newObsTable.push_back(observationTable[0]);
    observationTable = newObsTable;
    extendedTable = std::vector<std::vector<std::vector<bool>>>();

    accessSeqs = std::vector<Trace>();
    extdAccessSeqs = std::vector<std::vector<Trace>>();
    orientation = std::vector<bool>();

    rowFinder = BiTree();
}

void LStar::warmStartColumns(int l){

    Trace empty = Trace();
    extendColumns(empty);

    std::vector<int> powers;
    for(int i=0; i < l; i++){
        int power = (int)std::pow(alphabetSize,i);
        powers.push_back(power);
    }

    for(int tL=1; tL <= l; tL++){
        //generate and test all sequences of length tW
        for(int cCount=0; cCount < std::pow(alphabetSize,tL); cCount++){
            std::vector<int> tlSeq;
            for(int wc=tL-1; wc >= 0; wc--){
                int digit = (cCount / powers[wc]) % alphabetSize;
                tlSeq.push_back(digit);
            }
            Trace newCol = Trace(-1, tlSeq);
            extendColumns(newCol);
        }
    }

}

bool LStar::testClosed(int &unclosedRow, int &unclosedChar)
{
    for(int i = 0; i < nRowsS; i++){
        for(int c = 0; c < alphabetSize; c++){
            if(rowFinder.find(extendedTable[i][c]) == -1){
                unclosedRow = i;
                unclosedChar = c;
                return false;
            }
        }
    }
    return true;
}

bool LStar::testConsistent(std::vector<int> &inconsistent){
    //Not working with the current implementation since all rows in S are assumed unique
        for(int i = 0; i < nRowsS; i++){
            for(int j = i+1; j < nRowsS; j++){
                for(int c = 0; c < alphabetSize; c++){
                    for(int m = 0; m < nColumns; m++){
                        if(extendedTable[i][c][m] != extendedTable[j][c][m]){
                            inconsistent.push_back(i);
                            inconsistent.push_back(j);
                            inconsistent.push_back(c);
                            inconsistent.push_back(m);
                            return false;
                        }
                    }
                }
            }
        }
    return true;
}

int LStar::scoreDirection(std::vector<Trace> &extensions, std::vector<std::vector<bool>> &observations){
    int score = 0;
    extdAccessSeqs.push_back(extensions);
    for(int c=0; c < alphabetSize; c++){
        std::vector<bool> rowObs;
        for(int i=0; i < nColumns; i++){
            bool obs = queryModel(nRowsS-1, i, true, c);
            rowObs.push_back(obs);
        }
        if(rowFinder.find(rowObs) > -1){
            score++;
        }
        observations.push_back(rowObs);
    }
    extdAccessSeqs.pop_back();

    return score;
}

bool LStar::determineDirection(int accessSeqId, std::vector<Trace> &newExtensions, 
                               std::vector<std::vector<bool>> &queryResults){
    std::vector<Trace> leftExtensions = std::vector<Trace>();
    std::vector<Trace> rightExtensions = std::vector<Trace>();
    buildNewExtensions(accessSeqId, true, leftExtensions);
    buildNewExtensions(accessSeqId, false, rightExtensions);

    std::vector<std::vector<bool>> leftObservations;
    std::vector<std::vector<bool>> rightObservations;
    int leftScore = scoreDirection(leftExtensions, leftObservations);
    int rightScore = scoreDirection(rightExtensions, rightObservations);

    if(leftScore == rightScore){
        std::uniform_int_distribution<int> dist(0,1);
        bool dir = dist(gen);
        if(preferLeft || dir){
            newExtensions = leftExtensions;
            queryResults = leftObservations;
            return true;
        }else{
            newExtensions = rightExtensions;
            queryResults = rightObservations;
            return false;
        }
        
    }

    if(leftScore > rightScore){
        newExtensions = leftExtensions;
        queryResults = leftObservations;
        return true;
    }else{
        newExtensions = rightExtensions;
        queryResults = rightObservations;
        return false;
    }
}

void LStar::buildNewExtensions(int accessSeqId, bool orientation, std::vector<Trace> &newExtensions){
    int centre = 0;
    if(orientation){
        centre = 1;
    }
    newExtensions = std::vector<Trace>();
    for(int c = 0; c < alphabetSize; c++){
        std::vector<int> charV = std::vector<int>{c};
        Trace cTrace = Trace(-1, charV, centre);
        Trace newAccess = accessSeqs[accessSeqId].extend(cTrace);
        newExtensions.push_back(newAccess);
    }
}

bool LStar::queryModel(int rowID, int colID, bool extd, int c){
    Trace access= Trace();
    if(!extd){
        access = accessSeqs[rowID];
    }else{
        access = extdAccessSeqs[rowID][c];
    }
    Trace unknown = access.extend(endSeqs[colID]);
    return teacher.memberQuery(unknown);
}

void LStar::extendRows(int accessSeqId, int charId){

    nRowsS++;
    accessSeqs.push_back(extdAccessSeqs[accessSeqId][charId]);
    observationTable.push_back(extendedTable[accessSeqId][charId]);
    rowFinder.insert(extendedTable[accessSeqId][charId], nRowsS-1);
    
    std::vector<Trace> newExtensions;
    std::vector<std::vector<bool>> observations;
    bool newOri = determineDirection(nRowsS-1, newExtensions, observations);
    orientation.push_back(newOri);

    extdAccessSeqs.push_back(newExtensions);
    extendedTable.push_back(observations);
}

void LStar::extendColumns(Trace& newEndSeq){
    endSeqs.push_back(newEndSeq);
    nColumns++;
    for(int i = 0; i < nRowsS; i++){
        bool nField = queryModel(i, nColumns-1, false);
        observationTable[i].push_back(nField);
        rowFinder.insert(observationTable[i], i);
        for(int c = 0; c < alphabetSize; c++){
            bool nExtdField = queryModel(i, nColumns-1, true, c);
            extendedTable[i][c].push_back(nExtdField);
        }
    }
}

Trace LStar::processCounterexample(Trace& counterexample){
    int cLeft = 0;
    int cRight = 0;
    int cRow = 0;
    bool foundDisting = false;
    std::vector<int> tmp;
    counterexample.vectorise(tmp);
    //One trace for traversing (reads are not reset)
    Trace curCounter( -1, tmp);
    //One trace for comparing (reads will be reset)
    Trace cCountComp( -1, tmp);

    while(!foundDisting){
        bool cDirection = hypotheses.getOrientation(cRow);
        if(cDirection){
            cLeft++;
        }else{
            cRight++;
        }
        if(cLeft + cRight > curCounter.length){
            throw std::runtime_error("No distinguishing sequence found in counterexample.");
        }
        cRow = hypotheses.followEdge(cRow, curCounter.nextChar(cDirection, true));
        if(!cCountComp.equalUpTo(accessSeqs[cRow], cLeft, cRight)){
            //If a substitution is possible it is made and the trace results pre and post subsitution
            //are compared.
            Trace nextCounter = accessSeqs[cRow].extend(curCounter, false, cLeft, cRight);
            bool rPreSub = teacher.memberQuery(cCountComp);
            bool rPostSub = teacher.memberQuery(nextCounter);
            if(rPreSub != rPostSub){
                //If results differ the remainder of the sequence forms a distinguishing sequence
                foundDisting = true;
                curCounter.vectorise(tmp);
                tmp = std::vector<int>(tmp.begin()+cLeft, tmp.end()-cRight);
                Trace distingSeq(-1, tmp);
                return distingSeq;
            }else{
                //If results are the same the routine is repeated with the now substituted sequence
                curCounter = nextCounter;
                nextCounter.vectorise(tmp);
                cCountComp = Trace(-1, tmp);
                nextCounter.resetRead();
                cLeft = 0;
                cRight = 0;
                cRow = 0;
            }
        }
            //If no substitution is possible we continue processing the trace
    }

    return Trace();
}

/// @brief Creates a BiDFA model based on the observation table. State ids match corresponding row ids
/// @param hypotheses 
void LStar::modelFromTable(){

    std::vector<Node> nodes;

    for(int i=0; i < nRowsS; i++){

        std::vector<int> transitions;

        for(int c=0; c < alphabetSize; c++){

            int next = rowFinder.find(extendedTable[i][c]);

            if(next == -1){
                throw std::runtime_error("Attempted model construction with unclosed table.");
            }

            transitions.push_back(next);
        }

        Node rowNode = Node(observationTable[i][0], transitions, orientation[i]);
        nodes.push_back(rowNode);
    }

    hypotheses = BiDFA(alphabetSize, nRowsS, nodes);

}

bool LStar::wMethodEquiv(Trace& counterexample){

    std::vector<int> powers;
    for(int i=0; i < w; i++){
        int power = (int)std::pow(alphabetSize,i);
        powers.push_back(power);
    }

    for(int i=0; i < nRowsS; i++){
        for(int c=0; c < alphabetSize; c++){
            Trace acces = extdAccessSeqs[i][c];
            for(int tW=1; tW <= w; tW++){
                //generate and test all sequences of length tW
                for(int cCount=0; cCount < std::pow(alphabetSize,tW); cCount++){
                    std::vector<int> centreSeq;
                    int centre = 0;
                    int cNode = hypotheses.followTrace(acces);
                    for(int wc=tW-1; wc >= 0; wc--){
                        int digit = (cCount / powers[wc]) % alphabetSize;
                        centreSeq.push_back(digit);
                    }
                    //Identify the correct centre of the w lenght sequence
                    int left = 0;
                    int right = tW-1;
                    while(left <= right){
                        if(hypotheses.getOrientation(cNode)){
                            centre ++;
                            cNode = hypotheses.followEdge(cNode, centreSeq[left]);
                            left ++;
                        }else{
                            cNode = hypotheses.followEdge(cNode, centreSeq[right]);
                            right--;
                        }
                    }
                    Trace middle = Trace(-1, centreSeq, centre);
                    for(int j = 0; j < nColumns; j++){
                        Trace end = endSeqs[j];
                        Trace full = acces.extend(middle);
                        full.extend(end, true);
                        bool hypPred = hypotheses.parseTrace(full);
                        bool teachPred = teacher.memberQuery(full);
                        if(hypPred != teachPred){
                            counterexample = full;
                            //early stop if a counterexample is found
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

LStar::LStar(int alphabetSizeI, Model &pTeacher) : teacher(pTeacher), rowFinder(BiTree())
{

    if(alphabetSizeI == -1){
        throw std::runtime_error("Tried to initialise without providing size of the alphabet.");
    }
    this->alphabetSize = alphabetSizeI;
    setParameters();

    nRowsS = 0;
    nColumns = 0;

    firstRun = true;
    isFitted = false;
}

void LStar::setParameters(bool useW, int w, int earlyStop, int randomSeed, bool preferLeft,
                          int warmStart, bool resetRemember){

    this->useW = useW;
    this->w = w;
    this->earlyStop = earlyStop;
    this->fullEarlyStop = earlyStop;
    this->resetRemember = resetRemember;
    this->preferLeft = preferLeft;
    this->warmStart = warmStart;
    gen = std::default_random_engine(randomSeed);

    if(resetRemember){
        this->earlyStop = 8;
        if(this->fullEarlyStop < this->earlyStop){
            this->earlyStop = this->fullEarlyStop;
        }
    }

}

void LStar::fit(){

    bool modelFitted = false;

    while(earlyStop <= fullEarlyStop){

        initialise();

        while(!modelFitted && nRowsS < earlyStop){
            bool closed = false;
            while(!closed){
                int unclosedRow;
                int unclosedChar;
                closed = testClosed(unclosedRow, unclosedChar);
                if(!closed){
                    extendRows(unclosedRow, unclosedChar);
                }
            }
            modelFromTable();
            Trace counterexample;
            modelFitted = wMethodEquiv(counterexample);
            if(!modelFitted){
                Trace distSeq;
                distSeq = processCounterexample(counterexample);
                extendColumns(distSeq);
            }
        }
        //handle information transfer between reset runs
        if(!remember()){
            break;
        }else{
            reset();
        }
        modelFitted = false;
    }
    std::cout << "Finished fitting model. \n";
    isFitted = true;
}

void LStar::getFittedModel(BiDFA &model){

    if(!isFitted){
        throw std::runtime_error("Tried to return model before fitting.");
    }else{
        model = bestHypotheses;
    }
}