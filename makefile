
CXX = g++
CXXFLAGS_BASE = -Wall -std=c++17 -O3

N ?= 10000
THREADS ?= 10
EXECS ?= 49
CSV_FILE ?= scheduling_dinamico.csv

app_test: main.cpp
	$(CXX) $(CXXFLAGS_BASE) -fopenmp -o AutP_test main.cpp

app_seq: main.cpp
	$(CXX) $(CXXFLAGS_BASE) -o AutP_seq main.cpp

app_csv: main.cpp
	$(CXX) $(CXXFLAGS_BASE) -fopenmp -DCSV_OUTPUT -o AutP_csv main.cpp

app_csv_seq: main.cpp
	$(CXX) $(CXXFLAGS_BASE) -DCSV_OUTPUT -o AutS_csv main.cpp

test: app_test
	.\AutP_test $(N) $(THREADS) 1

testS: app_seq
	@echo "Attenzione: Esecuzione sequenziale senza OpenMP."
	.\AutP_seq $(N) 1 1

run: app_csv
	@echo "Avvio simulazione..."
	.\AutP_csv $(N) $(THREADS) $(EXECS) >> $(CSV_FILE)
	@echo "Risultati salvati in $(CSV_FILE)"

runS: app_csv_seq
	@echo "Avvio simulazione ..."
	.\AutP_csv $(N) 1 $(EXECS) >> $(CSV_FILE)
	@echo "Risultati salvati in $(CSV_FILE)"

clean:
	rm -f AutP_test.exe AutP_seq.exe AutP_csv.exe $(CSV_FILE)