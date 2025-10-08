#include <bits/stdc++.h>
using namespace std;

// run2: decoder skeleton with helper stubs
// Usage: run2 <basename>.city <basename>.satoutput <basename>.metromap <basename>.vars
//
// This file contains parser helpers and decoding stubs. Implement the var-file parser
// and mapping from positive assigned literals back to X/E/T/R variables and then
// reconstruct each line's path.

// Read vars file mapping "id name" lines into a map
unordered_map<int,string> readVarMap(const string &varsfile){
    unordered_map<int,string> mp;
    ifstream f(varsfile);
    if(!f) return mp;
    string line;
    while(getline(f,line)){
        if(line.size()==0 || line[0]=='#') continue;
        // simple parse: look for first token (id) and rest is name
        stringstream ss(line);
        int id; if(!(ss>>id)) continue;
        string name; getline(ss,name);
        // trim
        if(name.size() && name[0]==' ') name = name.substr(1);
        mp[id]=name;
    }
    return mp;

}

// Parse minisat output. Return pair(status, assigned lits vector)
pair<string, vector<int>> parseMiniSatOutput(const string &satoutput){
    ifstream f(satoutput);
    if(!f) throw runtime_error("Cannot open satoutput");
    string status;
    f>>status;
    vector<int> lits;
    int v;
    while(f>>v) lits.push_back(v);
    return {status, lits};
}

// Reconstruct paths: using var map and assigned positive literals, build for each k the
// list of moves by following E variables from start. This is a stub explaining where to
// implement logic.
void reconstructPaths(const string &cityfile, const unordered_map<int,string> &varmap,
                      const vector<int> &assignedLits, const string &outpath){
    // TODO: parse cityfile to get N,M,K and starts/ends
    // TODO: build a quick dictionary of positive var ids
    unordered_set<int> pos;
    for(int lit: assignedLits) if(lit>0) pos.insert(lit);

    ifstream in(cityfile);
    int scen; in>>scen;
    int N,M,K,J,P=0;
    if(scen==1) in>>N>>M>>K>>J; else in>>N>>M>>K>>J>>P;
    vector<array<int,4>> lines(K);
    for(int i=0;i<K;i++) in>>lines[i][0]>>lines[i][1]>>lines[i][2]>>lines[i][3];
    // skip popular cells if any
    if(scen==2) for(int i=0;i<P;i++){int a,b; in>>a>>b;}

    ofstream out(outpath);
    if(!out) throw runtime_error("Cannot write metromap");

    // Placeholder: write empty path for each line. Replace with traversal logic.
    for(int k=0;k<K;k++){
        out<<"0\n";
    }
}

int main(int argc, char** argv){
    if(argc!=5){
        cerr<<"Usage: "<<argv[0]<<" <in.city> <in.satoutput> <out.metromap> <varsfile>\n";
        return 1;
    }
    string cityfile=argv[1];
    string satoutput=argv[2];
    string metromap=argv[3];
    string varsfile=argv[4];

    auto varmap = readVarMap(varsfile);
    auto res = parseMiniSatOutput(satoutput);
    string status = res.first;
    if(status=="UNSAT"){
        ofstream out(metromap);
        out<<"0\n";
        cerr<<"run2: problem UNSAT, wrote 0 to "<<metromap<<"\n";
        return 0;
    }
    reconstructPaths(cityfile, varmap, res.second, metromap);
    cerr<<"run2: wrote skeleton metromap to "<<metromap<<"\n";
    return 0;
}
