#ifndef MODELS_FOR_SEQUENCE_PARSING
#define MODELS_FOR_SEQUENCE_PARSING

#include <vector>
#include <iostream>
#include <string>

typedef short int ternary;

struct Trace{

    int length;
    int centre;
    ternary accepted;

    /// @brief Returns the next character in the specified reading direction
    /// @param direction True - left to right, False - right to left
    /// @param ignoreCentre If false will stop reading when crossing the centre
    /// @return the next character
    int nextChar(bool direction, bool ignoreCentre=false);

    void resetRead();

    void vectorise(std::vector<int>& vTrace);

    Trace extend(Trace& interfix, bool inplace=false, int pLeft=0, int pRight=0);

    bool equalUpTo(Trace& otherTrace, int pLeft, int pRight, bool ignoreCentre=true);

    Trace();

    Trace(ternary accepted, std::vector<int>& characters, int centre=-1);

    private:
        std::vector<int> characters;

        int currentPosLeft;
        int currentPosRight;

};


struct Model{

    std::string mType = "Base";
    
    virtual bool memberQuery(Trace& trace);
    virtual bool equivQuery(Model& model, Trace& counterexample);

};

struct Node{
    
    ternary accepting;
    /// @brief True - extends left to right on left side; False - extends right to left on right side
    bool orientation;

    int followEdge(int c);

    Node(ternary accepting, std::vector<int>& transitions, bool orientation=true);

    private:
        std::vector<int> transitions;

};



class BiDFA: public Model{

    int nNodes;
    int nCharacters;

    /// @brief List of states. Id 0 is always the initial state
    std::vector<Node> nodes;

    public:

        BiDFA();

        BiDFA(int nCharacters, int nNodes, std::vector<Node>& nodes);

        int getSize();

        ternary parseTrace(Trace& trace);
        int followEdge(int node, int cChar);
        int followTrace(Trace& trace);
        bool getOrientation(int node);
        bool memberQuery (Trace& trace) override;
        bool equivQuery(Model& model, Trace& counterexample) override;

        void print(std::ostream& out);

};

#endif