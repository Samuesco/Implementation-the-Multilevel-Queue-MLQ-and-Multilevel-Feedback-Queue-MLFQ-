#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <fstream>
using namespace std;
struct Proceso {
    int pid;               // Process ID
    int arrival;           // Arrival Time
    int BT;                // Burst Time
    int BTrestante;        // Remaining Burst Time
    int start;             // Start Time
    int finish;            // Finish Time
    bool started = false;  // Flag to check if process has started
    int priority;          // Priority of the process
    int waitTime;          // Total accumulated waiting time
    int lastExecutedTime;  // Last time the process was executed
};
vector<int> split(const string& cadena) {
    vector<int> resultado;
    string tempo = "";
    int i = 0;
    while (i < cadena.size()) {
        if (cadena[i] == ' ') {
            if (!tempo.empty()) {
                resultado.push_back(stoi(tempo));
                tempo = ""; 
            }
        }else {
            tempo += cadena[i];
        }
        i++;
    }
    return resultado;
}

vector<vector<int>>guardarinstrucciones(const string& nombreArchivo){
    vector<vector<int>> resultado;
    ifstream archivo(nombreArchivo);
    string linea;
    if (archivo.is_open()) {
        while (getline(archivo, linea)) {
            vector<int> instruccion = split(linea);
            resultado.push_back(instruccion);
        }
        archivo.close();
    } else {
        cout << "No se pudo abrir el archivo." << std::endl;
    }
    return resultado;
}
bool compareArrival(const Proceso &a, const Proceso &b) {
    return a.arrival < b.arrival;
}

bool compareBurstTime(const Proceso &a, const Proceso &b) {
    return a.BTrestante < b.BTrestante;  // Sort by shortest job first
}
void RoundRobinMLFQ(queue<Proceso> q_rr, int timeOS, int quantum, vector<Proceso> finished,queue<Proceso> q_rrnext){
    Proceso p = q_rr.front();
    q_rr.pop();
    if (p.started == false) {
        p.start = timeOS;
        p.started = true;
        p.lastExecutedTime = timeOS - p.arrival;
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
}
void RoundRobinMLFQ2(queue<Proceso> q_rr2, int timeOS, int quantum, vector<Proceso> finished,queue<Proceso> q_rr1,queue<Proceso> q_rrnext){
    Proceso p = q_rr2.front();
    if (p.started == false) {
        p.start = timeOS;
        p.started = true;
        p.lastExecutedTime = timeOS;
    }else{
        p.waitTime += timeOS - p.lastExecutedTime;
    }
    int timeSlice = 0;
    while (timeSlice < quantum && p.BTrestante > 0) {
        if (!q_rr1.empty() && q_rr1.front().arrival <= timeOS) {
            p.lastExecutedTime = timeOS;
            return;  // Interrumpir y ejecutar el proceso de mayor prioridad
        }
        p.BTrestante--;
        timeOS++;
        timeSlice++;
    }
    q_rr2.pop();
    if (p.BTrestante == 0) {
        p.finish = timeOS;
        finished.push_back(p);
    }else {
        p.priority++;
        q_rrnext.push(p);  // Reincerta en la cola siguiente
    }        

}
void RoundRobinMLQ(queue<Proceso> q_rr, int timeOS, int quantum, vector<Proceso> finished) {
    Proceso p = q_rr.front();
    q_rr.pop();
    if (p.started == false) {
        p.start = timeOS;
        p.started = true;
        p.lastExecutedTime = timeOS - p.arrival;
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

void ShortestJobFirst(vector<Proceso> q_sjf, int timeOS, vector<Proceso> finished) {
    sort(q_sjf.begin(), q_sjf.end(), compareBurstTime);// Sort by shortest job first
    Proceso p = q_sjf.front();
    q_sjf.erase(q_sjf.begin());
    if (p.started == false) {
        p.start = timeOS;
        p.started = true;
        p.waitTime = timeOS - p.arrival;
    }
    timeOS += p.BTrestante;
    p.BTrestante = 0;
    p.finish = timeOS;
    finished.push_back(p);      
}


void ShortestJobFirstMLQ(vector<Proceso> q_sjf, int timeOS, vector<Proceso> finished,queue<Proceso> q_rr) {
    sort(q_sjf.begin(), q_sjf.end(), compareBurstTime);// Sort by shortest job first
    Proceso p = q_sjf.front();
    if (p.started == false) {
        p.start = timeOS;
        p.started = true;
        p.waitTime = timeOS - p.arrival;
    }else{
        p.waitTime += timeOS - p.lastExecutedTime;
    }
    while (p.BTrestante > 0) {
        // Chequear si llega un proceso de mayor prioridad
        if (!q_rr.empty() && q_rr.front().arrival <= timeOS) {
            p.lastExecutedTime = timeOS;
            return;  // Interrumpir y ejecutar el proceso de mayor prioridad
        }
        timeOS++;
        p.BTrestante--;
        p.lastExecutedTime = timeOS;
    }
    q_sjf.erase(q_sjf.begin());
    p.finish = timeOS;
    finished.push_back(p);
}


void FCFSMFLQ(queue<Proceso> q_fcfs, int timeOS, vector<Proceso> finished,queue<Proceso> q_rr1 ,queue<Proceso> q_rr2) {
    Proceso p = q_fcfs.front();
    if (!p.started) {
        p.start = timeOS;
        p.started = true;
        p.waitTime = timeOS - p.arrival;
    }else{
        p.waitTime += timeOS - p.lastExecutedTime;
    }
    while (p.BTrestante > 0) {
        // Chequear si llega un proceso de mayor prioridad
        if ((!q_rr1.empty() || !q_rr2.empty()) && (q_rr1.front().arrival == timeOS || q_rr2.front().arrival == timeOS)){
            p.lastExecutedTime = timeOS;
            return;  // Interrumpir y ejecutar el proceso de mayor prioridad
        }
        // Ejecutar el proceso y actualizar tiempos
        timeOS++;
        p.BTrestante--;
        p.lastExecutedTime = timeOS;
    }
    q_fcfs.pop();
    p.finish = timeOS;
    finished.push_back(p);
}

void FCFSMLQ(queue<Proceso> q_fcfs, int timeOS, vector<Proceso> finished,queue<Proceso> q_rr ,vector<Proceso> q_sjf) {
    Proceso p = q_fcfs.front();
    if (!p.started) {
        p.start = timeOS;
        p.started = true;
        p.waitTime = timeOS - p.arrival;
    }else{
        p.waitTime += timeOS - p.lastExecutedTime;
    }
    while (p.BTrestante > 0) {
        // Chequear si llega un proceso de mayor prioridad
        if ((!q_rr.empty() || !q_sjf.empty()) && (q_rr.front().arrival == timeOS || q_sjf[0].arrival == timeOS)){
            p.lastExecutedTime = timeOS;
            return;  // Interrumpir y ejecutar el proceso de mayor prioridad
        }
        // Ejecutar el proceso y actualizar tiempos
        timeOS++;
        p.BTrestante--;
        p.lastExecutedTime = timeOS;
    }
    q_fcfs.pop();
    p.finish = timeOS;
    finished.push_back(p);
}


void MLQ(vector<Proceso> procesos, int quantumRR) {
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
    float totalWaitingTime = 0, totalTurnaroundTime = 0;
    for (Proceso &p : finished) {
        int waitingTime = p.waitTime;
        int turnaroundTime = p.finish - p.arrival;
        totalWaitingTime += waitingTime;
        totalTurnaroundTime += turnaroundTime;
        cout << "Process " << p.pid << " executed from " << p.start << " to " << p.finish
             << " with waiting time: " << waitingTime << endl;
    }

    cout << "Waiting Time: " << totalWaitingTime  << endl;
    cout << "Turnaround Time: " << totalTurnaroundTime  << endl;
}

void MLFQ(vector<Proceso>procesos, int quantumRR1,int quantumRR2){
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
    while (!q_rr1.empty() || !q_rr2.empty() || !q_fcfs.empty()) {

        if (!q_rr1.empty() && q_rr1.front().arrival <= timeOS) {
            RoundRobinMLFQ(q_rr1,timeOS,quantumRR1,finished,q_rr2);
        }
        else if (!q_rr2.empty() && q_rr2.front().arrival <= timeOS) {
            RoundRobinMLFQ(q_rr2,timeOS,quantumRR2,finished,q_fcfs);
        }
        //  First Come First Served queue
        else if (!q_fcfs.empty()&& q_fcfs.front().arrival <= timeOS) {
            FCFSMFLQ(q_fcfs,timeOS,finished,q_rr1,q_rr2);
        }
    }
    // Calculate and display average waiting time and turnaround time
    float totalWaitingTime = 0, totalTurnaroundTime = 0;
    for (Proceso &p : finished) {
        int waitingTime = p.waitTime;
        int turnaroundTime = p.finish - p.arrival;
        totalWaitingTime += waitingTime;
        totalTurnaroundTime += turnaroundTime;
        cout << "Process " << p.pid << " executed from " << p.start << " to " << p.finish;
        cout << " with waiting time: " << waitingTime << endl;
    }

    cout << "Waiting Time: " << totalWaitingTime << endl;
    cout << "Turnaround Time: " << totalTurnaroundTime << endl;

}
vector<Proceso> retornarArrayProcesos(){
    vector<vector<int>>entrada = guardarinstrucciones("procesos.txt");
    vector<Proceso> procesos;
    for (int i = 0; i < entrada.size(); i++){
        Proceso newp;
        newp.pid = entrada[i][0];
        newp.arrival = entrada[i][1];
        newp.BT = entrada[i][2];
        newp.priority = entrada[i][3];
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
        opcion = op[0]- '0';
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