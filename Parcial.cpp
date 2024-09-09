#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <fstream>
#include <typeinfo> 

using namespace std;
struct Proceso {
    int pid;               // Process ID
    int arrival;           // Arrival Time
    int BT;                // Burst Time
    int BTrestante;        // Remaining Burst Time
    int start = 0;             // Start Time / response time
    int finish = 0;            // Finish Time
    bool started = false;  // Flag to check if process has started
    int priority;          // Priority of the process
    int waitTime = 0;          // Total accumulated waiting time
    int lastExecutedTime = 0;  // Last time the process was executed
    int quantumres = 0;
};
void imprimirProceso(const Proceso& p) {
    cout << p.pid << " "
              << p.arrival << " "
              << p.BT << " "
              << p.BTrestante << " "
              << p.priority << " "
              << p.start << " "
              << p.finish << " "
              << p.started << " "
              << p.waitTime << " "
              << p.lastExecutedTime << " "
              << p.quantumres << endl;
}
vector<string> split(const std::string& cadena) {
    vector<string> resultado;
    string tempo = "";
    int i = 0;
    while (i < cadena.size()) {
        if (cadena[i] == ' ') {
            if (!tempo.empty()) {
                resultado.push_back(tempo);
                tempo = ""; 
            }
        }else {
            tempo += cadena[i];
        }
        i++;
    }if (!tempo.empty()) {
        resultado.push_back(tempo);
    }

    return resultado;
}

vector<vector<string>>guardarinstrucciones(const string nombreArchivo){
    vector<vector<string>> resultado;
    ifstream archivo(nombreArchivo);
    string linea;
    if (archivo.is_open()) {
        while (getline(archivo, linea)) {
            vector<string> instruccion = split(linea);
            resultado.push_back(instruccion);
        }
        archivo.close();
    } else {
        cout << "No se pudo abrir el archivo." << endl;
    }
    return resultado;
}
bool compareArrival(const Proceso &a, const Proceso &b) {
    return a.arrival < b.arrival;
}

bool compareBurstTime(const Proceso &a, const Proceso &b) {
    return a.BTrestante < b.BTrestante;  // Sort by shortest job first
}
int RoundRobinMLFQ(queue<Proceso> &q_rr, int &timeOS, int quantum, vector<Proceso> &finished,queue<Proceso> &q_rrnext){
    Proceso p = q_rr.front();
    q_rr.pop();
    if (p.started == false){
        p.start = timeOS;
        p.started = true;
        p.lastExecutedTime = timeOS;
    }
    int timeSlice = min(quantum, p.BTrestante);
    p.waitTime += timeOS - p.lastExecutedTime;
    timeOS += timeSlice; 
    if(p.BTrestante - timeSlice <= 0){
        p.BTrestante = 0;
    }else{
        p.lastExecutedTime = timeOS;
        p.BTrestante -= timeSlice;
    }
    if (p.BTrestante == 0) {
        p.finish = timeOS;
        finished.push_back(p);
    }else{
        p.priority++;
        q_rrnext.push(p);  // Reincerta en la cola siguiente
    }
    return 0;        
}
int RoundRobinMLFQ2(queue<Proceso> &q_rr2, int &timeOS, int quantum, vector<Proceso> &finished,queue<Proceso> &q_rr1,queue<Proceso> &q_rrnext){
    int timeSlice = 0;
    if (q_rr2.front().started == false) {
        q_rr2.front().start = timeOS;
        q_rr2.front().started = true;
        q_rr2.front().lastExecutedTime = timeOS;
        q_rr2.front().waitTime+= timeOS - q_rr2.front().arrival;
    }else{
        q_rr2.front().waitTime += timeOS - q_rr2.front().lastExecutedTime;
        timeSlice = q_rr2.front().quantumres;
    }
    while (timeSlice < quantum && q_rr2.front().BTrestante > 0) {
        if (!q_rr1.empty() && q_rr1.front().arrival <= timeOS){
            q_rr2.front().quantumres = timeSlice;
            q_rr2.front().lastExecutedTime = timeOS;
            return 1;  // Interrumpir y ejecutar el proceso de mayor prioridad
        }
        q_rr2.front().BTrestante--;
        timeOS++;
        timeSlice++;
    }
    
    if (q_rr2.front().BTrestante == 0) {
        q_rr2.front().finish = timeOS;
        finished.push_back(q_rr2.front());
    }else {
        q_rr2.front().priority++;
        q_rrnext.push(q_rr2.front());  // Reincerta en la cola siguiente
    }
    q_rr2.pop();
    return 0;        
}
void RoundRobinMLQ(queue<Proceso> &q_rr, int &timeOS, int quantum, vector<Proceso> &finished) {
    Proceso p = q_rr.front();
    q_rr.pop();
    if (p.started == false) {
        p.start = timeOS;
        p.started = true;
        p.lastExecutedTime = timeOS;
    }
    int timeSlice = min(quantum, p.BTrestante);
    p.waitTime += timeOS - p.lastExecutedTime;
    timeOS += timeSlice; 
    if(p.BTrestante - timeSlice <= 0){
        p.BTrestante = 0;
    }else{
        p.lastExecutedTime = timeOS;
        p.BTrestante -= timeSlice;
    }
    if (p.BTrestante == 0) {
        p.finish = timeOS;
        finished.push_back(p);
    }else {
        q_rr.push(p);  // Reincerta en la cola si falta 
    }            
}


void ShortestJobFirstMLQ(vector<Proceso> &q_sjf, int &timeOS, vector<Proceso> &finished,queue<Proceso> &q_rr) {
    sort(q_sjf.begin(), q_sjf.end(), compareBurstTime);// Sort by shortest job first
    if (q_sjf.front().started == false) {
        q_sjf.front().start = timeOS;
        q_sjf.front().started = true;
        q_sjf.front().waitTime = timeOS - q_sjf.front().arrival;
    }else{
        q_sjf.front().waitTime += timeOS - q_sjf.front().lastExecutedTime;
    }
    while (q_sjf.front().BTrestante > 0) {
        // Chequear si llega un proceso de mayor prioridad
        if (!q_rr.empty() && q_rr.front().arrival <= timeOS) {
            q_sjf.front().lastExecutedTime = timeOS;
            return;  // Interrumpir y ejecutar el proceso de mayor prioridad
        }
        timeOS++;
        q_sjf.front().BTrestante--;
        q_sjf.front().lastExecutedTime = timeOS;
    }
    q_sjf.front().finish = timeOS;
    finished.push_back(q_sjf.front());
    q_sjf.erase(q_sjf.begin());
}



int FCFSMFLQ(queue<Proceso> &q_fcfs, int &timeOS, vector<Proceso> &finished,queue<Proceso> &q_rr1 ,queue<Proceso> &q_rr2) {
    if (!q_fcfs.front().started){
        q_fcfs.front().start = timeOS;
        q_fcfs.front().started = true;
        q_fcfs.front().waitTime = timeOS - q_fcfs.front().arrival;
    }else{
        q_fcfs.front().waitTime += timeOS - q_fcfs.front().lastExecutedTime;
    }
    while (q_fcfs.front().BTrestante > 0) {
        // Chequear si llega un proceso de mayor prioridad
        if ((!q_rr1.empty() || !q_rr2.empty()) && (q_rr1.front().arrival == timeOS || q_rr2.front().arrival == timeOS)){
            q_fcfs.front().lastExecutedTime = timeOS;
            return 1;  // Interrumpir y ejecutar el proceso de mayor prioridad
        }
        // Ejecutar el proceso y actualizar tiempos
        timeOS++;
        q_fcfs.front().BTrestante--;
        q_fcfs.front().lastExecutedTime = timeOS;
    }
    q_fcfs.front().finish = timeOS;
    finished.push_back(q_fcfs.front());
    q_fcfs.pop();
    return 0;
}

void FCFSMLQ(queue<Proceso> &q_fcfs, int &timeOS, vector<Proceso> &finished,queue<Proceso> &q_rr ,vector<Proceso> &q_sjf) {
    if (!q_fcfs.front().started) {
        q_fcfs.front().start = timeOS;
        q_fcfs.front().started = true;
        q_fcfs.front().waitTime = timeOS - q_fcfs.front().arrival;
    }else{
        q_fcfs.front().waitTime += timeOS - q_fcfs.front().lastExecutedTime;
    }
    while (q_fcfs.front().BTrestante > 0) {
        // Chequear si llega un proceso de mayor prioridad
        if ((!q_rr.empty() || !q_sjf.empty()) && (q_rr.front().arrival == timeOS || q_sjf[0].arrival == timeOS)){
            q_fcfs.front().lastExecutedTime = timeOS;
            return;  // Interrumpir y ejecutar el proceso de mayor prioridad
        }
        // Ejecutar el proceso y actualizar tiempos
        timeOS++;
        q_fcfs.front().BTrestante--;
        q_fcfs.front().lastExecutedTime = timeOS;
    }
    q_fcfs.front().finish = timeOS;
    finished.push_back(q_fcfs.front());
    q_fcfs.pop();
}


void MLQ(vector<Proceso> &procesos, int quantumRR) {
    int n = procesos.size();
    queue<Proceso> q_rr;                 // Queue for Round Robin
    vector<Proceso> q_sjf;               // Vector for Shortest Job First
    queue<Proceso> q_fcfs;               // Queue for First Come First Served
    vector<Proceso> finished;
    int timeOS = 0;
    // Sort processes by arrival time
    sort(procesos.begin(), procesos.end(), compareArrival);

    int index = 0;
    // Agregar los procesos a sus respectivas Colas
    while (index < n ) {
            if (procesos[index].priority == 1) {
                q_rr.push(procesos[index]);  // Round Robin queue
            } else if (procesos[index].priority == 2) {
                q_sjf.push_back(procesos[index]);  // Shortest Job First queue
            } else {
                q_fcfs.push(procesos[index]);  // First Come First Served queue
            }
            index++;
    }
    sort(q_sjf.begin(), q_sjf.end(), compareBurstTime); 
    // Continue until all processes are finished
    while (!q_rr.empty() || !q_sjf.empty() || !q_fcfs.empty()) {

        if (!q_rr.empty() && q_rr.front().arrival <= timeOS) {
            RoundRobinMLQ(q_rr,timeOS,quantumRR,finished);
        }
        // Shortest Job First queue
        else if (!q_sjf.empty() && q_sjf[0].arrival <= timeOS) {
            ShortestJobFirstMLQ(q_sjf,timeOS,finished,q_rr);
        }
        //  First Come First Served queue
        else if (!q_fcfs.empty()&& q_fcfs.front().arrival <= timeOS) {
            FCFSMLQ(q_fcfs,timeOS,finished,q_rr,q_sjf);
        }
    }

    // Calculate and display average waiting time and turnaround time
    float totalWaitingTime = 0, totalTurnaroundTime = 0, totalCT = 0, totalrt = 0;
    for (int i = 0 ; i < finished.size(); i++) {
        int waitingTime = finished[i].waitTime;
        int turnaroundTime = finished[i].finish - finished[i].arrival;
        totalWaitingTime += waitingTime;
        totalTurnaroundTime += turnaroundTime;
        totalCT += finished[i].finish;
        totalrt+=finished[i].start;
        cout << "Process " << finished[i].pid << " executed from " << finished[i].start << " to " << finished[i].finish<< endl;
        cout << "Waiting time: " << waitingTime << endl;
        cout << "Turnaround time: "<< turnaroundTime << endl;
        cout << "Response Time: " << finished[i].start << endl<< endl;
    }
    cout << "________________________________________________________________________________________________"<<endl;
    cout << "Waiting Time PROM: " << totalWaitingTime/finished.size()  << endl;
    cout << "Turnaround Time PROM: " << totalTurnaroundTime/finished.size()  << endl;
    cout << "Response Time PROM: " << totalCT/finished.size()  << endl;
    cout << "Complete Time PROM: " << totalrt/finished.size()  << endl;
}
void imprimirCola(const queue<Proceso>& q, const string& nombreCola) {
    cout << "Cola " << nombreCola << ":" << endl;
    queue<Proceso> copia = q;
    while (!copia.empty()) {
        cout << copia.front().pid << " "<< copia.front().arrival<< " " << copia.front().BT << endl;
        copia.pop();
    }
}
void MLFQ(vector<Proceso>&procesos, int quantumRR1,int quantumRR2){
    int n = procesos.size();
    queue<Proceso> q_rr1;                 // Queue for Round Robin 1
    queue<Proceso> q_rr2;                 // Queue for Round Robin 2
    queue<Proceso> q_fcfs;               // Queue for First Come First Served
    vector<Proceso> finished;
    int timeOS = 0;
    sort(procesos.begin(), procesos.end(), compareArrival);
    int index = 0;
    // Agregar los procesos a sus respectivas Colas
    while (index < n ) {
            if (procesos[index].priority == 1) {
                q_rr1.push(procesos[index]);  // Round Robin queue 1
            } else if (procesos[index].priority == 2) {
                q_rr2.push(procesos[index]);  // Round Robin queue 2
            } else {
                q_fcfs.push(procesos[index]);  // First Come First Served queue
            }
            index++;
    }
    while (!q_rr1.empty() || !q_rr2.empty() || !q_fcfs.empty()){
        if (!q_rr1.empty() && q_rr1.front().arrival <= timeOS){
            int pr1 = RoundRobinMLFQ(q_rr1,timeOS,quantumRR1,finished,q_rr2);
        }
        else if (!q_rr2.empty() && q_rr2.front().arrival <= timeOS) {
            int pr2 = RoundRobinMLFQ2(q_rr2,timeOS,quantumRR2,finished,q_rr1,q_fcfs);
        }
        //  First Come First Served queue
        else if (!q_fcfs.empty()&& q_fcfs.front().arrival <= timeOS) {
            int pr3 = FCFSMFLQ(q_fcfs,timeOS,finished,q_rr1,q_rr2);
        }
    }
    // Calculate and display average waiting time and turnaround time
    float totalWaitingTime = 0, totalTurnaroundTime = 0, totalCT = 0, totalrt = 0;
    for (int i = 0 ; i < finished.size(); i++) {
        int waitingTime = finished[i].waitTime;
        int turnaroundTime = finished[i].finish - finished[i].arrival;
        totalWaitingTime += waitingTime;
        totalTurnaroundTime += turnaroundTime;
        totalCT += finished[i].finish;
        totalrt+=finished[i].start;
        cout << "Process " << finished[i].pid << " executed from " << finished[i].start << " to " << finished[i].finish<< endl;
        cout << "Waiting time: " << waitingTime << endl;
        cout << "Turnaround time: "<< turnaroundTime << endl;
        cout << "Response Time: " << finished[i].start << endl<< endl;
    }
    cout << "________________________________________________________________________________________________"<<endl;
    cout << "Waiting Time PROM: " << totalWaitingTime/finished.size()  << endl;
    cout << "Turnaround Time PROM: " << totalTurnaroundTime/finished.size()  << endl;
    cout << "Response Time PROM: " << totalCT/finished.size()  << endl;
    cout << "Complete Time PROM: " << totalrt/finished.size()  << endl;


}
vector<Proceso> retornarArrayProcesos(){
    vector<vector<string>>entrada = guardarinstrucciones("procesos.txt");
    vector<Proceso> procesos;
    for (int i = 0; i < entrada.size(); i++){
        Proceso newp;
        newp.pid = stoi(entrada[i][0]);
        newp.arrival = stoi(entrada[i][1]);
        newp.BT = stoi(entrada[i][2]);
        newp.BTrestante = stoi(entrada[i][2]);
        newp.priority = stoi(entrada[i][3]);
        procesos.push_back(newp);
    }
    return procesos;
}
void menu(){
    string op;
    int opcion;
    vector<Proceso> procesos = retornarArrayProcesos(); 
    do {    
        cout << "Menú de opciones:" << endl;
        cout << "1. MLFQ (Multi-Level Feedback Queue)" << endl;
        cout << "2. MLQ (Multi-Level Queue)" << endl;
        cout << "3. Salir" << endl;
        cout << "Selecciona una opción: ";
        getline(cin, op);
        opcion = op[0] - '0';
        switch(opcion) {
            case 1:
                MLFQ(procesos,2,4);
                break;
            case 2:
                MLQ(procesos,2);
                break;
            case 3:
                cout << "Saliendo del programa" << endl;
                break;
            default:
                cout << "Opción no válida, por favor selecciona nuevamente." << endl;
                break;
            }
    }while(opcion != 3);
}
int main(){
    menu();
    return 0;
}