#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <cstdint>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

// La nuova NextGeneration processa 64 celle alla volta usando Algebra Booleana
void NextGeneration(const vector<uint64_t>& CurrentGen, vector<uint64_t>& NextGen, int N, int words_per_row, int& chunk)
{
    // Il chunk rimane, ma ora ogni "blocco" in X gestisce 64 celle!
    #pragma omp parallel for schedule(guided, 50) 
    for(int y = 1; y < N - 1; ++y)
    {
        // Partiamo da 1 e arriviamo a words_per_row - 1 per evitare i bordi laterali
        for(int x = 1; x < words_per_row - 1; ++x)
        {
            int idx = y * words_per_row + x;

            // 1. CARICAMENTO DEL BLOCCO CENTRALE E DEI VICINI (64 celle a variabile!)
            uint64_t C = CurrentGen[idx];       // Centro
            uint64_t L = CurrentGen[idx - 1];   // Blocco di Sinistra
            uint64_t R = CurrentGen[idx + 1];   // Blocco di Destra

            // Spostamento dei bit per ottenere Est e Ovest (fondendo i bordi dei blocchi vicini)
            uint64_t W = (C >> 1) | (L << 63);
            uint64_t E = (C << 1) | (R >> 63);

            // Riga Superiore
            uint64_t NC = CurrentGen[(y - 1) * words_per_row + x];
            uint64_t NL = CurrentGen[(y - 1) * words_per_row + x - 1];
            uint64_t NR = CurrentGen[(y - 1) * words_per_row + x + 1];
            uint64_t NW = (NC >> 1) | (NL << 63);
            uint64_t NE = (NC << 1) | (NR >> 63);

            // Riga Inferiore
            uint64_t SC = CurrentGen[(y + 1) * words_per_row + x];
            uint64_t SL = CurrentGen[(y + 1) * words_per_row + x - 1];
            uint64_t SR = CurrentGen[(y + 1) * words_per_row + x + 1];
            uint64_t SW = (SC >> 1) | (SL << 63);
            uint64_t SE = (SC << 1) | (SR >> 63);

            // 2. IL MICRO-SOMMATORE HARDWARE (Calcola i vicini per 64 celle simultaneamente)
            // Usiamo 4 "strati" di bit per tenere traccia del conteggio (da 0 a 8 vicini)
            uint64_t bit0 = 0, bit1 = 0, bit2 = 0, bit3 = 0;

            // Funzione Lambda che implementa un "Full Adder" digitale
            auto add_neighbors = [&](uint64_t val) {
                uint64_t c0 = bit0 & val; bit0 ^= val;
                uint64_t c1 = bit1 & c0;  bit1 ^= c0;
                uint64_t c2 = bit2 & c1;  bit2 ^= c1;
                bit3 ^= c2;
            };

            // Sommiamo tutti gli 8 vicini (NIENTE '+', solo porte logiche!)
            add_neighbors(NW); add_neighbors(NC); add_neighbors(NE);
            add_neighbors(W);                     add_neighbors(E);
            add_neighbors(SW); add_neighbors(SC); add_neighbors(SE);

            // 3. APPLICAZIONE DELLE REGOLE DELLA VITA (Sempre per 64 celle insieme)
            // E' vivo se ha esattamente 3 vicini -> (bit1=1, bit0=1) e bit2,bit3 = 0
            uint64_t is_3 = bit0 & bit1 & ~bit2 & ~bit3;
            // Ha 2 vicini -> (bit1=1, bit0=0) e bit2,bit3 = 0
            uint64_t is_2 = ~bit0 & bit1 & ~bit2 & ~bit3;

            // Nuovo stato = Nasce (se 3) OPPURE Sopravvive (se era già 1 e ha 2 vicini)
            NextGen[idx] = is_3 | (C & is_2);
        }
    }
}

int main(int argc, char* argv[]){
    srand(42);
    int N = 5000;
    int num_threads = 4;
    int iter = 100;
    int executions = 1;
    int chunk = 50;
    string opt = "-O3";
   
    if(argc >= 5){
        N = stoi(argv[1]);
        num_threads = stoi(argv[2]);
        executions = stoi(argv[3]);
        opt = argv[4];
    }

    #ifdef _OPENMP
    omp_set_num_threads(num_threads);
    #else
    num_threads = 1;
    #endif
    
    N = ((N + 63) / 64) * 64; 
    int words_per_row = N / 64; 
    
    // Griglia compattata: invece di N*N interi, allochiamo (N * words_per_row) interi a 64-bit
    // Riduzione drastica della memoria usata!
    vector<uint64_t> CurrentGen(N * words_per_row, 0);
    vector<uint64_t> Gen(N * words_per_row, 0);
    vector<uint64_t> NextGen(N * words_per_row, 0);

    // Inizializzazione Bit a Bit
    for(int y = 1; y < N - 1; ++y) {
        for(int x = 1; x < N - 1; ++x) {
            if(rand() % 2 == 1) {
                int word_x = x / 64;       // In quale blocco da 64 si trova la cella?
                int bit_idx = x % 64;      // Qual è il suo bit specifico (da 0 a 63)?
                // Accendiamo il singolo bit usando l'operatore OR
                Gen[y * words_per_row + word_x] |= (1ULL << bit_idx);
            }
        }
    }

    for(int j = 0; j < executions; j++){
        CurrentGen = Gen;
        #ifndef CSV_OUTPUT
            cout << "Parte la simulazione SWAR (Run " << j << ").." << endl;
        #endif
        
        auto start_time = chrono::high_resolution_clock::now();
        
        for(int i = 0; i <= iter; ++i) {
            NextGeneration(CurrentGen, NextGen, N, words_per_row, chunk);
            swap(CurrentGen, NextGen);
        }

        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end_time - start_time;

        #ifdef CSV_OUTPUT
            cout << N << "," << num_threads << "," << iter << "," << j << "," << elapsed.count() << "," << opt << endl;
        #else
            cout << "Simulazione SWAR completata!" << endl;
            cout << "Matrice effettiva " << N << "x" << N << endl;
            cout << "Tempo: " << elapsed.count() << " secondi" << endl;
        #endif
    }

    return 0;
}