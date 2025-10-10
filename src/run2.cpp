#include <bits/stdc++.h>
using namespace std;

// Decoder for grid-edge encoding: follow true E(k,x,y,d) from start to end and emit directions
// Usage: run2 <basename>.city <basename>.satoutput <basename>.metromap <varsfile>
// Note: varsfile is ignored; we recompute variable IDs via the same VarMapper scheme as run1.

static inline bool inBounds(int N,int M,int x,int y){ return x>=0 && x<N && y>=0 && y<M; }
static inline vector<tuple<int,int,int>> neighbors4(int N,int M,int x,int y){
    vector<tuple<int,int,int>> res;
    if(x+1<N) res.emplace_back(0,x+1,y); // R
    if(x-1>=0) res.emplace_back(1,x-1,y); // L
    if(y+1<M) res.emplace_back(2,x,y+1); // D
    if(y-1>=0) res.emplace_back(3,x,y-1); // U
    return res;
}
static inline char dirChar(int d){ return d==0?'R' : d==1?'L' : d==2?'D' : 'U'; }
static inline int oppositeDir(int d){ return d==0?1 : d==1?0 : d==2?3 : 2; }

struct VarMapper {
    int N,M,K;
    int base_X, base_E, base_T, base_R;
    void init(int N_,int M_,int K_,int base){
        N=N_; M=M_; K=K_;
        base_X = base;
        base_E = base_X + K*(N*M);
        base_T = base_E + K*(N*M*4);
        base_R = base_T + K*(N*M);
    }
    inline int var_X(int k,int x,int y) const { return base_X + k*(N*M) + y*N + x; }
    inline int var_E(int k,int x,int y,int d) const { return base_E + k*(N*M*4) + (y*N + x)*4 + d; }
};

// Robust minisat output parser: handles SAT/SATISFIABLE and v-lines; collects ints
static bool parseMiniSat(const string &path, vector<int>& lits, bool &isSAT){
    ifstream f(path);
    if(!f) return false;
    isSAT = false;
    string tok;
    while(f>>tok){
        if(tok=="SAT" || tok=="SATISFIABLE") { isSAT = true; continue; }
        if(tok=="UNSAT" || tok=="UNSATISFIABLE") { isSAT = false; lits.clear(); return true; }
        if(tok=="v" || tok=="0") continue;
        try{
            int v = stoi(tok);
            lits.push_back(v);
        }catch(...){
            // ignore non-integers
        }
    }
    return true;
}

int main(int argc, char** argv){
    if(argc!=5){
        cerr<<"Usage: "<<argv[0]<<" <in.city> <in.satoutput> <out.metromap> <varsfile>\n";
        return 1;
    }
    string cityfile=argv[1];
    string satoutput=argv[2];
    string metromap=argv[3];
    // string varsfile=argv[4]; // not needed when we recompute ids

    // Parse city
    ifstream in(cityfile);
    if(!in){ cerr<<"Cannot open "<<cityfile<<"\n"; return 2; }
    int scen; in>>scen;
    int N,M,K,J,P=0; if(scen==1) in>>N>>M>>K>>J; else in>>N>>M>>K>>J>>P;
    vector<array<int,4>> lines(K);
    for(int i=0;i<K;i++) in>>lines[i][0]>>lines[i][1]>>lines[i][2]>>lines[i][3];
    if(scen==2){ for(int i=0;i<P;i++){ int a,b; in>>a>>b; } }

    // Parse SAT output
    vector<int> allLits; bool sat;
    if(!parseMiniSat(satoutput, allLits, sat)){
        cerr<<"Cannot read "<<satoutput<<"\n"; return 3;
    }
    ofstream out(metromap);
    if(!out){ cerr<<"Cannot write "<<metromap<<"\n"; return 4; }
    if(!sat){ out<<"0\n"; return 0; }

    // Build positive literal set
    unordered_set<int> pos; pos.reserve(allLits.size()*2);
    for(int lit: allLits) if(lit>0) pos.insert(lit);

    // Prepare var mapper same as encoder
    VarMapper vm; vm.init(N,M,K,1);

    // For each line, follow edges from start to end
    for(int k=0;k<K;k++){
        int sx=lines[k][0], sy=lines[k][1];
        int ex=lines[k][2], ey=lines[k][3];
        int cx=sx, cy=sy;
        int px=-1, py=-1; // previous cell
        vector<char> dirs; dirs.reserve(N*M*2);

        // Safety cap to avoid infinite loops if assignment is inconsistent
        int maxSteps = N*M*4;
        int steps=0;
        while(!(cx==ex && cy==ey) && steps<maxSteps){
            steps++;
            // Gather incident true edges at (cx,cy)
            vector<pair<int,pair<int,int>>> candidates; // (dir,(nx,ny))
            auto nbs = neighbors4(N,M,cx,cy);
            for(auto &tp: nbs){
                int d,nx,ny; tie(d,nx,ny)=tp;
                int e = vm.var_E(k,cx,cy,d);
                if(pos.count(e)) candidates.push_back({d,{nx,ny}});
            }

            if(candidates.empty()){
                // No outgoing edge — inconsistent; emit 0 for this line and break
                dirs.clear();
                break;
            }

            // Choose next: prefer the candidate that is not coming back from previous
            int chosenDir = -1, nx=cx, ny=cy;
            if(px==-1){
                // at start: expect exactly one
                chosenDir = candidates[0].first;
                nx = candidates[0].second.first; ny = candidates[0].second.second;
                if(candidates.size()>=2){
                    // if two present at start (shouldn't), pick the one that leads closer to end by manhattan (fallback)
                    int best=candidates[0].first, bx=nx, by=ny; int bestd=abs(nx-ex)+abs(ny-ey);
                    for(size_t i=1;i<candidates.size();++i){
                        int tx=candidates[i].second.first, ty=candidates[i].second.second;
                        int d2=abs(tx-ex)+abs(ty-ey);
                        if(d2<bestd){ best=candidates[i].first; bx=tx; by=ty; bestd=d2; }
                    }
                    chosenDir=best; nx=bx; ny=by;
                }
            }else{
                // internal: two edges expected; pick the one different from prev cell
                for(auto &cnd: candidates){
                    int tx=cnd.second.first, ty=cnd.second.second;
                    if(!(tx==px && ty==py)){ chosenDir=cnd.first; nx=tx; ny=ty; break; }
                }
                if(chosenDir==-1){
                    // Both point back to prev? pick first as fallback
                    chosenDir=candidates[0].first; nx=candidates[0].second.first; ny=candidates[0].second.second;
                }
            }

            dirs.push_back(dirChar(chosenDir));
            px=cx; py=cy; cx=nx; cy=ny;
        }

        // Output this line
        if(dirs.empty() && !(sx==ex && sy==ey)){
            // Inconsistent; emit just 0
            out<<"0\n";
        } else {
            for(size_t i=0;i<dirs.size();++i){
                out<<dirs[i];
                if(i+1<dirs.size()) out<<" ";
                else out<<" ";
            }
            out<<"0\n";
        }
    }

    return 0;
}
