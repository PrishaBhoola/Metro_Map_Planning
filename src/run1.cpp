#include <bits/stdc++.h>
using namespace std;

// run1: encoder skeleton with helper stubs
// Usage: run1 <basename>.city <basename>.satinput <basename>.vars
//
// This file contains small, well-documented helper functions you'll want to fill in
// when implementing the real CNF encoding. The main() below shows the I/O flow and
// where to call the helpers.

struct CNF {
    vector<vector<int>> clauses;
    int nextVar = 1; // next free variable id (for aux vars)

    int newVar(){ return nextVar++; }
    void addClause(const vector<int>& c){ clauses.push_back(c); }
    void addUnit(int a){ clauses.push_back(vector<int>{a}); }
    // helper to write DIMACS
    void writeDIMACS(const string &path){
        ofstream f(path);
        if(!f) throw runtime_error("Cannot open satoutput file");
        f<<"p cnf "<<nextVar-1<<" "<<clauses.size()<<"\n";
        for(auto &c: clauses){
            for(int lit: c) f<<lit<<" ";
            f<<"0\n";
        }
    }
};

// -----------------------------------------------------------------------------
// Variable id helper functions
// You should reserve ranges for each variable kind and implement these functions.
// Example mapping conventions are in ENCODING.md. Fill the base offsets before use.
// -----------------------------------------------------------------------------

struct VarMapper {
    int N, M, K;
    int base_X, base_E, base_T, base_R; // assign at runtime

    // call this once you choose base offsets (e.g., base_X = 1)
    void init(int N_, int M_, int K_, int base){
        N = N_; M = M_; K = K_;
        base_X = base;
        base_E = base_X + K*(N*M);
        base_T = base_E + K*(N*M*4);
        base_R = base_T + K*(N*M);
    }

    inline int var_X(int k,int x,int y){ // cell used by line k
        return base_X + k*(N*M) + y*N + x;
    }
    inline int var_E(int k,int x,int y,int d){ // d: 0=R,1=L,2=D,3=U
        return base_E + k*(N*M*4) + (y*N + x)*4 + d;
    }
    inline int var_T(int k,int x,int y){
        return base_T + k*(N*M) + y*N + x;
    }
    inline int var_R(int k,int x,int y){
        return base_R + k*(N*M) + y*N + x;
    }
};

// -----------------------------------------------------------------------------
// Cardinaity: Sinz sequential counter helper (skeleton)
// Input: list of literals L, enforce sum(L) <= Kbound. Returns vector of aux var ids
// You'll need to allocate aux variables from CNF::nextVar (or by a passed function).
// -----------------------------------------------------------------------------

// Adds clauses implementing Sum(L) <= bound using Sinz's sequential counter.
// This is a skeleton: implement the exact clause pattern from the paper.
void addSinzAtMost(CNF &cnf, const vector<int> &L, int bound){
    // TODO: implement Sinz sequential counter.
    // Steps (high level):
    // 1) if bound >= L.size() -> nothing to do
    // 2) create auxiliary variables s[i][j]
    // 3) add clauses as in Sinz construction
    // Use cnf.newVar() to allocate aux vars and cnf.addClause(...) to append clauses
}

// -----------------------------------------------------------------------------
// Helper to add pairwise at-most-one for small sets (simple but O(n^2) clauses)
// -----------------------------------------------------------------------------
void addPairwiseAtMostOne(CNF &cnf, const vector<int> &vars){
    for(size_t i=0;i<vars.size();++i)
        for(size_t j=i+1;j<vars.size();++j)
            cnf.addClause(vector<int>{-vars[i], -vars[j]});
}

// -----------------------------------------------------------------------------
// Higher-level clause builders you will use for the encoding
// - linkEdgeCell: E -> X(target) and E -> X(source) clauses
// - enforceDegree: degree clauses for start/end/internal cells
// - detectTurn: add clauses that set T(k,x,y) when used edges are orthogonal
// -----------------------------------------------------------------------------

void linkEdgeCell(CNF &cnf, VarMapper &vm, int k,int x,int y,int d){
    int e = vm.var_E(k,x,y,d);
    // compute target coordinates (x2,y2)
    int x2=x, y2=y, od;
    if(d==0){ x2=x+1; y2=y; od=1; } else if(d==1){ x2=x-1; y2=y; od=0; }
    else if(d==2){ x2=x; y2=y+1; od=3; } else { x2=x; y2=y-1; od=2; }
    int xs = vm.var_X(k,x,y);
    int xt = vm.var_X(k,x2,y2);
    int eopp = vm.var_E(k,x2,y2,od);
    // E -> X(source)
    cnf.addClause(vector<int>{-e, xs});
    // E -> X(target)
    cnf.addClause(vector<int>{-e, xt});
    // E -> E(opposite)
    cnf.addClause(vector<int>{-e, eopp});
}

// Placeholder: add other builders per plan in ENCODING.md

// -----------------------------------------------------------------------------
// Main: parse input and call helpers to generate CNF
// -----------------------------------------------------------------------------
int main(int argc, char** argv){
    if(argc!=4){
        cerr<<"Usage: "<<argv[0]<<" <in.city> <out.satinput> <out.vars>\n";
        return 1;
    }
    string cityfile=argv[1];
    string satfile=argv[2];
    string varsfile=argv[3];

    ifstream in(cityfile);
    if(!in){ cerr<<"Cannot open "<<cityfile<<"\n"; return 2; }

    int scen;
    in>>scen;
    int N,M,K,J,P=0;
    if(scen==1){
        in>>N>>M>>K>>J;
    } else {
        in>>N>>M>>K>>J>>P;
    }
    vector<array<int,4>> lines(K);
    for(int i=0;i<K;i++){
        in>>lines[i][0]>>lines[i][1]>>lines[i][2]>>lines[i][3];
    }
    vector<pair<int,int>> popular;
    if(scen==2){
        for(int i=0;i<P;i++){ int x,y; in>>x>>y; popular.emplace_back(x,y);}    
    }

    // Prepare CNF and variable mapper. Choose base offset 1 for X.
    CNF cnf;
    VarMapper vm; vm.init(N,M,K,1);
    // Update nextVar in CNF to account for reserved variable IDs
    cnf.nextVar = vm.base_R + K*(N*M) ; // after R block

    // TODO: generate clauses using the helpers above
    // Example: add one unit clause for debugging
    cnf.addUnit(1);

    // write DIMACS and vars mapping
    cnf.writeDIMACS(satfile);
    ofstream vout(varsfile);
    vout<<"# Variable map (skeleton)\n";
    vout<<"Base_X="<<vm.base_X<<" Base_E="<<vm.base_E<<" Base_T="<<vm.base_T<<" Base_R="<<vm.base_R<<"\n";
    vout.close();

    cerr<<"run1: wrote skeleton CNF to "<<satfile<<" (replace with real encoding)\n";
    return 0;
}
