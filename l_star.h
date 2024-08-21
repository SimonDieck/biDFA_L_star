#ifndef L_STAR_FOR_BIDFA
#define L_STAR_FOR_BIDFA

#include "models.h"

#include <vector>
#include <utility>
#include <random>


typedef struct BiTree{
    private:
        int id=-1;
        struct BiTree * left = nullptr;
        struct BiTree * right = nullptr;
    public:
        BiTree();
        ~BiTree();

        int find(std::vector<bool>& bSeq, int currentPos=0);
        void insert(std::vector<bool>& bSeq, int rid, int currentPos=0);
        /// @brief Deprecated since all rows in observation table should be unique by definition
        /// @param rowBuckets 
        void getUniqueRows(std::vector<std::vector<int>>& rowBuckets);

} BiTree;


class LStar{

    int alphabetSize;
    //TODO: currently W method is mandatory as we don't have an equivalence query
    bool useW;
    int w;
    int earlyStop;
    bool preferLeft;
    int warmStart;
    //Parameters for running the algorithm with resets
    bool resetRemember;
    bool firstRun;
    int prevEarlyStop = 0;
    int fullEarlyStop;
    
    bool isFitted = false;

    std::default_random_engine gen;

    Model& teacher;

    BiDFA hypotheses;
    BiDFA bestHypotheses;

    std::vector<Trace> accessSeqs;
    std::vector<std::vector<Trace>> extdAccessSeqs;
    std::vector<Trace> endSeqs;

    std::vector<bool> orientation;

    int nRowsS;
    int nColumns;

    std::vector<std::vector<bool>> observationTable;
    std::vector<std::vector<std::vector<bool>>> extendedTable;

    BiTree rowFinder;

    void initialise();

    bool remember();
    void reset();

    void warmStartColumns(int l);

    /// @brief Test if the observation table is closed, if not return the id of the unmatched row
    /// @param unclosedRow id of row in the observation table with unmatched successor
    /// @param unclosedChar id of the the character that leads to no match
    /// @return True if table is closed, false otherwise
    bool testClosed(int& unclosedRow, int &unclosedChar);

    /// @brief Deprecated, since we assume rows in S to be unqiue. --
    /// Checks whether the observation table is consistent 
    /// @param inconsistent vector of length 4: 0 and 1 contain ids of inconsistent rows, 
    /// 2 the id of the character; and 3 id of end sequence that lead to inconsistency
    /// @return True if table consistent, false otherwise
    bool testConsistent(std::vector<int>& inconsistent);

    int scoreDirection(std::vector<Trace>& extensions, std::vector<std::vector<bool>>& observations);

    bool determineDirection(int accessSeqId, std::vector<Trace>& newExtensions, 
                            std::vector<std::vector<bool>>& queryResults);

    void buildNewExtensions(int accessSeqId, bool orientation, std::vector<Trace>& newExtensions);

    bool queryModel(int rowID, int colID, bool extd, int c=-1);

    void extendRows(int accessSeqId, int charId);

    void extendColumns(Trace& newEndSeq);

    Trace processCounterexample(Trace& counterexample);

    void modelFromTable();

    bool wMethodEquiv(Trace& counterexample);

    public:

        LStar(int alphabetSizeI, Model& pTeacher);

        void setParameters(bool useW=true, int w=3, int earlyStop = 3000, int randomSeed=24, 
                           bool preferLeft=true, int warmStart=0, bool resetRemember=false);

        void fit();

        void getFittedModel(BiDFA& model);

};


#endif