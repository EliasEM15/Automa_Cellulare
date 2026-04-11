#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

void NextGeneration (const vector<int>& CurrentGen, vector<int>& NextGen, int N)
{
    #pragma omp parallel for firstprivate(CurrentGen)
  

    for(int y=1; y< N - 1; ++y)
    {
        for(int x=1; x < N-1; ++x)
        {
            int Alive=0;

            for(int dy=-1; dy <= 1; ++dy)
            {
                for(int dx=-1; dx <= 1; ++dx)
                {

                    if(dx==0 && dy==0)
                    {
                        continue;
                    }
             
                    Alive += CurrentGen[(y+dy)*N + (x+dx)];

                }

            }
            int CurrentState= CurrentGen[y*N + x];

            if(Alive<2 || Alive>3)
            {
                NextGen[y*N + x]=0;
            }
            else if(CurrentState==0 && Alive==3)
            {
                NextGen[y*N + x]=1;
            }
            else
            {
                NextGen[y*N + x]=CurrentState;
            }
        }
    }
}


int main(int argc, char* argv[]){
    srand(42);
    string mode="test";
    int N=5000;
    int num_threads=4;
    int iter=100;
    int executions=1;
   
 
    if(argc>= 4){

        N=stoi(argv[1]);
        num_threads=stoi(argv[2]);
        executions=stoi(argv[3]);
    }
    // cout<<"N,Threads,Iterazioni,Esecuzione,Tempo_s"<<endl;

    #ifdef _OPENMP
    omp_set_num_threads(num_threads);
    #else
    num_threads = 1;
    #endif
    
    
    //griglia NxN
    vector<int> CurrentGen(N*N, 0);
    vector<int> Gen(N*N, 0);
    vector<int> NextGen(N*N, 0);
    for(int x=1; x < N-1; ++x)
    {
        for(int y=1; y < N-1; ++y)
        {
            Gen[x*N + y] = rand()%2;
        }
    };
for(int j=0; j <= executions; j++){

    CurrentGen=Gen;
    #ifndef CSV_OUTPUT
        cout<<"Parte la simulazione (Run "<< j <<").." << endl;
    #endif
    auto start_time = chrono::high_resolution_clock::now();
    

    for(int i=0; i <= iter; ++i)
    {
        NextGeneration(CurrentGen, NextGen, N);
        swap(CurrentGen, NextGen);
    }

 auto end_time = chrono::high_resolution_clock::now();

 

    chrono::duration<double> elapsed = end_time - start_time;

   #ifdef CSV_OUTPUT
     cout << N << "," << num_threads << "," << iter << "," << j << "," << elapsed.count() << endl;
   #else
    cout<<"simulazione completata"<<endl;
    cout<<"Matrice "<<N<<"x"<<N<<endl;
    cout<<"numero di iterazioni simulate: "<<iter<<endl;
    cout<<"tempo: "<< elapsed.count() << endl;
    cout<<"numero di threads: "<< num_threads << endl;
    #endif
}



}