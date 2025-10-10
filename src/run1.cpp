#include <bits/stdc++.h>
using namespace std;

struct CNF{

    vector<vector<int>>clauses;
    int next_var =1;

    int new_var(){
        return next_var++;
    }
    void add_clause(const vector<int>& c){
        clauses.push_back(c);
    }
    void add_unit(int a){
        clauses.push_back(vector<int>{a});
    }
    void write_DIMACS(const string& path){
        ofstream f(path);
        if(!f){
            throw runtime_error("cannot open satinput file");
        }
        f << "p cnf " << (next_var - 1) << " " << clauses.size() << "\n";
        for(const auto &c: clauses){
            for(int lit : c){
                f << lit << " ";
            }
            f << "0\n";
        }
    }

};


struct var_mapper{
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

    //line K uses (x,y)
    inline int var_X(int k,int x,int y){ // cell used by line k
        return base_X + k*(N*M) + y*N + x;
    }

    // line K uses edge from x,y in direction d: 0=R,1=L,2=D,3=U 
    inline int var_E(int k,int x,int y,int d){ // d: 0=R,1=L,2=D,3=U
        return base_E + k*(N*M*4) + (y*N + x)*4 + d;
    }

    // line K turns at cell x,y
    inline int var_T(int k,int x,int y){
        return base_T + k*(N*M) + y*N + x;
    }

    //cell (x,y) is rachable by line k from start along used edges
    inline int var_R(int k,int x,int y){
        return base_R + k*(N*M) + y*N + x;
    }

};

// this function chevk if the new coordinated are in the bounds of the board
bool in_bounds(int N,int M,int x,int y){
    return x>=0 && x<N && y>=0 &&  y<M;
}

// this fuctnion provides all the neighbouring coordiantes and the direction we moved in to reach there 
vector<tuple<int,int,int>>get_neighbours(int N,int M,int x,int y){
    vector<tuple<int,int,int>> res;
    //right
    if(x+1<N){
        res.push_back({0,x+1,y});
    }
    //left
    if(x-1>=0){
        res.push_back({1,x-1,y});
    }
    //down
    if(y+1<M){
        res.push_back({2,x,y+1});
    }
    //up
    if(y-1>=0){
        res.push_back({3,x,y-1});
    }

    return res;
}

//values of all four possibly moved edges from current cell 
vector<int> get_neighbour_values(int N,int M,int k_line,int x,int y,var_mapper vm){
    vector<int>incident;
    auto neighbours = get_neighbours(N,M,x,y);
    for(auto &tp: neighbours){
        int dir,nx,ny;
        tie(dir,nx,ny)=tp;
        incident.push_back(vm.var_E(k_line,x,y,dir));
    }
    return incident;
}

//now this function ensures the condition when an edge is formed that the train moves from a cell to another 
//the first clause saying if edges exist then source cell must be used
//the second says if edges exist then dest cell must be used
// the third says if front edge is marked to exist , reverse edge should also exist

void link_edge_cell(CNF &cnf,var_mapper &vm,int k,int x,int y,int d){
    int e = vm.var_E(k,x,y,d);

    int x2=x, y2=y, od;
    if(d==0){ x2=x+1; y2=y; od=1; } else if(d==1){ x2=x-1; y2=y; od=0; }
    else if(d==2){ x2=x; y2=y+1; od=3; } else { x2=x; y2=y-1; od=2; }
    int xs = vm.var_X(k,x,y);
    int xt = vm.var_X(k,x2,y2);
    int eopp = vm.var_E(k,x2,y2,od);

    cnf.add_clause(vector<int>{-e,xs});
    cnf.add_clause(vector<int>{-e,xt});
    cnf.add_clause(vector<int>({-e,eopp}));
}

// of all these edges there should eb only one edge that must exist 
// 1.) used for choosing the source and destiantion edge
void add_pairwise_atmost_one(CNF &cnf,const vector<int>& vars){
    int n = vars.size();
    for(int i=0;i<n;i++){
        for(int j=i+1;j<n;j++){
            cnf.add_clause(vector<int>{-vars[i],-vars[j]});
        }
    }
}

// add exactly one edges (manages taht of al the edges at most one of them is added)
void add_exactly_one(CNF &cnf,const vector<int> &vars){
    if(vars.empty()){
        return;
    }
    cnf.add_clause(vars);
    add_pairwise_atmost_one(cnf,vars);
}

//this function is to ensure that for a gicen cell only two moves are possible 

void add_guarded_exactly_two(CNF &cnf,int guard,const vector<int>& neigh_values){

    //ensures if metro was present there than any one of the edges exist
    vector<int>cl;
    cl.push_back(-guard);
    int val = neigh_values.size();
    for(int i=0;i<val;i++){
        cl.push_back(neigh_values[i]);
    }
    cnf.add_clause(cl);

    // this elevates the clause to at least two to be present
    for(int i=0;i<val;i++){
        vector<int> c; c.reserve(2+neigh_values.size()-1);
        c.push_back(-guard);
        c.push_back(-neigh_values[i]);
        for(size_t j=0;j<neigh_values.size();++j) if(j!=i) c.push_back(neigh_values[j]);
        cnf.add_clause(c);
    }

    //this prevents the presence of three moves of same block

    for(int i=0;i<val;i++){
        for(int j=i+1;j<val;j++){
            for(int k=j+1;k<val;k++){
                cnf.add_clause({-guard, -neigh_values[i], -neigh_values[j], -neigh_values[k]});
            }
        }
    }


}


//Enforce that you can’t reach j = bound at some prefix and then set another input true after that. 
//This prevents having more than bound true inputs overall.

void add_sinz_atmost(CNF &cnf,vector<int> &L,int bound){

    int n = L.size();
    if(bound >= n){
        return;
    }
    if(bound <= 0){
        for(int lit: L){
            cnf.add_clause({-lit});
        }
        return;
    }
    // Standard Sinz sequential counter for sum(L) <= bound
    // s[i][j] for i=1..n, j=1..bound
    vector<vector<int>> s(n+1, vector<int>(bound+1, 0));
    for(int i=1;i<=n;i++) for(int j=1;j<=bound;j++) s[i][j] = cnf.new_var();
    // Base for i=1
    cnf.add_clause({-L[0], s[1][1]}); // x1 -> s(1,1)
    // Transition for i=2..n
    for(int i=2;i<=n;i++){
        // j=1
        cnf.add_clause({-s[i-1][1], s[i][1]}); // s(i-1,1) -> s(i,1)
        cnf.add_clause({-L[i-1], s[i][1]});    // x_i -> s(i,1)
        // j=2..bound
        for(int j=2;j<=bound;j++){
            cnf.add_clause({-s[i-1][j], s[i][j]});               // s(i-1,j) -> s(i,j)
            cnf.add_clause({-L[i-1], -s[i-1][j-1], s[i][j]});    // x_i & s(i-1,j-1) -> s(i,j)
        }
    }
    // Overflow prevention: for i=1..n-1, not (s(i,bound) and x_{i+1})
    for(int i=1;i<=n-1;i++){
        cnf.add_clause({-s[i][bound], -L[i]});
    }

}


static inline int oppositeDir(int d){ return d==0?1 : d==1?0 : d==2?3 : 2; }

// function that the metro is reachable
//X→R forces every used cell to be marked reachable. 
//The justification clause forces any R(c) to be supported by an incoming edge from a reachable neighbor. 
//Together with degree constraints and E→X, this eliminates disconnected cycles.
void add_reachability_for_metro(CNF &cnf, var_mapper &vm, int k, int N, int M, int sx, int sy){
    // starting point presence of that line
    cnf.add_unit(vm.var_R(k, sx, sy));
    // if the k metro line has used tehcell then it is reachable 
    for(int y=0;y<M;y++) for(int x=0;x<N;x++) cnf.add_clause({-vm.var_X(k,x,y), vm.var_R(k,x,y)});
    // Justification of R
    for(int y=0;y<M;y++){
        for(int x=0;x<N;x++){
            if(x==sx && y==sy) continue;
            //non start cells
            vector<int> Avars;
            auto nbs = get_neighbours(N,M,x,y);
            for(auto &tp : nbs){
                int d_to_nb, nx, ny; tie(d_to_nb,nx,ny) = tp;
                int od = oppositeDir(d_to_nb); // from neighbor to (x,y)
                int e_in = vm.var_E(k, nx, ny, od);
                int Rn = vm.var_R(k, nx, ny);
                int A = cnf.new_var();
                cnf.add_clause({-A, Rn});
                cnf.add_clause({-A, e_in});
                cnf.add_clause({-Rn, -e_in, A});
                Avars.push_back(A);
            }
            vector<int> c; c.reserve(1+Avars.size());
            c.push_back(-vm.var_R(k,x,y));
            for(int a: Avars) c.push_back(a);
            cnf.add_clause(c);
        }
    }
}

//adding all the trun definition for the given parameter
void add_turn_defs(CNF &cnf, var_mapper &vm, int k, int x, int y){
    int eR = vm.var_E(k,x,y,0), eL = vm.var_E(k,x,y,1), eD = vm.var_E(k,x,y,2), eU = vm.var_E(k,x,y,3);
    int t = vm.var_T(k,x,y);
    cnf.add_clause({-t, eR, eL});
    cnf.add_clause({-t, eU, eD});
    cnf.add_clause({-eR, -eU, t});
    cnf.add_clause({-eR, -eD, t});
    cnf.add_clause({-eL, -eU, t});
    cnf.add_clause({-eL, -eD, t});
}


void add_turns_atmost_J(CNF &cnf, var_mapper &vm, int k, int N, int M, int J){
    vector<int> Ts; Ts.reserve(N*M);
    for(int y=0;y<M;y++) for(int x=0;x<N;x++) Ts.push_back(vm.var_T(k,x,y));
    add_sinz_atmost(cnf, Ts, J);
}


int main(int argc, char** argv){

    if(argc != 4){
        cerr<<"Usage: "<<argv[0]<<" <in.city> <out.satinput> <out.vars>\n";
        return 1;
    }

    string city_file = argv[1];
    string sat_file = argv[2];
    string vars_file = argv[3];

    ifstream in(city_file);
    if(!in){
        cerr <<"cannot open given file"<<endl;
        return 2;
    }

    int scenerio =1;
    int N,M,K,J,P=0;
    in >> scenerio;

    if(scenerio ==1){
        in>>N>>M>>K>>J;
    }
    else{
        in>>N>>M>>K>>J>>P;
    }

    vector<vector<int>>metro_lines(K,vector<int>(4));
    for(int i=0;i<K;i++){
        in>>metro_lines[i][0]>>metro_lines[i][1]>>metro_lines[i][2]>>metro_lines[i][3];
    }

    vector<pair<int,int>>popular_city;
    if(scenerio==2){
        for(int i=0;i<P;i++){
            int x,y;
            in>>x>>y;
            popular_city.push_back({x,y});
        }
    }


    CNF cnf;
    var_mapper vm;

    vm.init(N,M,K,1);
    cnf.next_var = vm.base_R + K*(N*M);

    //building constraints

    //constraint 1 ->  a cell can move if it is present in the consecutive possible cell 
    //all possible movements of the metro lines in four direction
    for(int kline=0;kline<K;kline++){
        for(int y=0;y<M;y++){
            for(int x=0;x<N;x++){
                for(int d=0; d<4; d++){
                   int nx =x;
                   int ny =y;
                   if(d==0){
                    nx = x+1;
                   }
                   else if(d==1){
                    nx = x-1;
                   }
                   else if(d==2){
                    ny = y+1;
                   }
                   else if(d==3){
                    ny = y-1;
                   }

                   if(!in_bounds(N,M,nx,ny)){
                    // it cannot move in that direction , since the direction is out of the board
                    cnf.add_clause({-vm.var_E(kline,x,y,d)});
                   }
                   else{
                    // o/w moved i that direction  i.e edge established
                    link_edge_cell(cnf,vm,kline,x,y,d);
                   }
                }
            }
        }
    }

    //constraint 2 -> starting and end point of the train lines

    for(int kline =0;kline<K;kline++){
        int sx = metro_lines[kline][0];
        int sy = metro_lines[kline][1];
        int ex = metro_lines[kline][2];
        int ey = metro_lines[kline][3];
        cnf.add_unit(vm.var_X(kline, sx, sy));
        cnf.add_unit(vm.var_X(kline, ex, ey));
    }

    //constraint 3-> 
    //1.) if the next movements gets it to its source or detination than move it there only 
    //2.) if it is not start or end then we ensure that for the given cell if the metro line was present exactly two of the moves are possible
    for(int kline =0;kline<K;kline++){
        int sx = metro_lines[kline][0];
        int sy = metro_lines[kline][1];
        int ex = metro_lines[kline][2];
        int ey = metro_lines[kline][3];
        for(int y=0;y<M;y++){
            for(int x=0;x<N;x++){
                vector<int> neigh_values = get_neighbour_values(N,M,kline,x,y,vm);
                if(x==sx && y==sy){
                    add_exactly_one(cnf,neigh_values);
                }
                else if(x==ex && y==ey){
                    add_exactly_one(cnf,neigh_values);
                }
                else{
                    int k_present = vm.var_X(kline,x,y);
                    add_guarded_exactly_two(cnf,k_present,neigh_values);
                }
            }
        }
    }

    //constraint 4-> to put that each cell can contain at most one metro line 
    for(int y=0;y<M;y++){
        for(int x=0;x<N;x++){
            vector<int>X_values;
            for(int klines=0;klines < K;klines++){
                X_values.push_back(vm.var_X(klines,x,y));
            }
            if(K<=8){
                //from all the k values on a given cell we can have atmost one of them
                add_pairwise_atmost_one(cnf,X_values);
            }
            else{
                // it ensures sum(X_values) <=1 using sinz sequential counter
                add_sinz_atmost(cnf,X_values,1);
            }
        }
    }

    //constraint 5-> if the cell is present it is reachable , prevents disconnected cycles
    for(int kline =0;kline<K;kline++){
        int sx= metro_lines[kline][0];
        int sy= metro_lines[kline][1];
        add_reachability_for_metro(cnf,vm,kline,N,M,sx,sy);
    }

    // C6 : turn detection and bound
    for(int kline=0;kline<K;kline++){
        for(int y=0;y<M;y++){
            for(int x=0;x<N;x++){
                add_turn_defs(cnf,vm,kline,x,y);
            }
        }
        add_turns_atmost_J(cnf,vm,kline,N,M,J);
    }

    // Scenario II: popular cells must be used by at least one metro line
    if(scenerio == 2){
        for(const auto &pp : popular_city){
            int px = pp.first, py = pp.second;
            vector<int> atleast_one;
            atleast_one.reserve(K);
            for(int kline=0;kline<K;kline++){
                atleast_one.push_back(vm.var_X(kline, px, py));
            }
            if(!atleast_one.empty()) cnf.add_clause(atleast_one);
        }
    }

    // write DIMACS and vars mapping
    cnf.write_DIMACS(sat_file);
    ofstream vout(vars_file);
    vout<<"# Variable map (blocks)\n";
    vout<<"Base_X="<<vm.base_X<<" Size_X="<<(K*(N*M))<<"\n";
    vout<<"Base_E="<<vm.base_E<<" Size_E="<<(K*(N*M*4))<<"\n";
    vout<<"Base_T="<<vm.base_T<<" Size_T="<<(K*(N*M))<<"\n";
    vout<<"Base_R="<<vm.base_R<<" Size_R="<<(K*(N*M))<<"\n";
    vout.close();

    cerr<<"run1: wrote CNF to "<<sat_file<<" (grid-edge encoding with reachability & turns)\n";
    return 0;


}