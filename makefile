
CXX = g++
CXXFLAGS_BASE = -Wall -std=c++17 $(OPT)

N ?= 10000
THREADS ?=128
EXECS ?= 50
CSV_FILE ?= Best_theoryP.csv
OPT ?= -O3 -march=native

app_test: main.cpp
	$(CXX) $(CXXFLAGS_BASE) -fopenmp -o AutP_test main.cpp

app_seq: main.cpp
	$(CXX) $(CXXFLAGS_BASE) -o AutP_seq main.cpp

app_csv: main.cpp
	$(CXX) $(CXXFLAGS_BASE) -fopenmp -DCSV_OUTPUT -o AutP_csv main.cpp

app_griglia: Automa_griglia.cpp
	$(CXX) $(CXXFLAGS_BASE) -fopenmp -DCSV_OUTPUT -o AutP_griglia Automa_griglia.cpp

app_csv_seq: main.cpp
	$(CXX) $(CXXFLAGS_BASE) -DCSV_OUTPUT -o AutS_csv main.cpp

app_bool: main_bool.cpp
	$(CXX) $(CXXFLAGS_BASE) -fopenmp -DCSV_OUTPUT -o AutBool_csv main_bool.cpp

test: app_test
	.\AutP_test $(N) $(THREADS) 1 "$(OPT)"

testS: app_seq
	@echo "Attenzione: Esecuzione sequenziale senza OpenMP."
	.\AutP_seq $(N) 1 1 "$(OPT)"

run: app_csv
	@echo "Avvio simulazione..."
	.\AutP_csv $(N) $(THREADS) $(EXECS) "$(OPT)" >> $(CSV_FILE)
	@echo "Risultati salvati in $(CSV_FILE)"

runS: app_csv_seq
	@echo "Avvio simulazione ..."
	.\AutS_csv $(N) 1 $(EXECS) "$(OPT)" >> $(CSV_FILE)
	@echo "Risultati salvati in $(CSV_FILE)"

runG: app_griglia
	@echo "Avvio simulazione ..."
	.\AutP_griglia $(N) $(THREADS) $(EXECS) "$(OPT)" >> $(CSV_FILE)
	@echo "Risultati salvati in $(CSV_FILE)"

runBool: app_bool
	@echo "Avvio simulazione ..."
	.\AutBool_csv $(N) $(THREADS) $(EXECS) "$(OPT)" >> $(CSV_FILE)
	@echo "Risultati salvati in $(CSV_FILE)"
clean:
	-del /Q /F AutP.exe AutP_test.exe AutP_seq.exe AutP_csv.exe AutP_griglia.exe AutS_csv.exe AutBool_csv.exe