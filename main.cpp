#include <iostream>
#include <string>
#include <cstdlib>
#include <signal.h>
#include <sched.h>
#include <sys/resource.h>
#include <unistd.h>
#include <cstring>
#include <iomanip>  // Para formatar a tabela

using namespace std;

void listarProcessos(const string& filtro) {
    string comando = "ps --no-header -eo comm,%cpu,etime,nlwp,psr,pid,user";

    // Incluir filtro, se houver
    if (!filtro.empty()) {
        comando += " | grep -i " + filtro;  // A opção -i torna a busca sem sensibilidade a maiúsculas/minúsculas
    }

    // Mostrar o cabeçalho da tabela
    cout << left << setw(20) << "Nome do Processo"
         << setw(10) << "% CPU"
         << setw(15) << "Tempo de Atividade"
         << setw(10) << "Threads"
         << setw(10) << "CPU"
         << setw(10) << "PID"
         << setw(20) << "Usuário" << endl;

    // Separação das colunas
    cout << string(95, '-') << endl;

    // Executar o comando para listar os processos
    system(comando.c_str());

    // Exibindo a % de uso da GPU, caso tenha uma GPU NVIDIA
    cout << "\nUso de GPU (se disponível):\n";
}

void matarProcesso(int pid) {
    if (kill(pid, SIGKILL) == -1) {
        perror("Erro ao matar o processo");
    } else {
        cout << "Processo " << pid << " finalizado." << endl;
    }
}

void suspenderProcesso(int pid) {
    if (kill(pid, SIGSTOP) == -1) {
        perror("Erro ao suspender o processo");
    } else {
        cout << "Processo " << pid << " suspenso." << endl;
    }
}

void continuarProcesso(int pid) {
    if (kill(pid, SIGCONT) == -1) {
        perror("Erro ao continuar o processo");
    } else {
        cout << "Processo " << pid << " retomado." << endl;
    }
}

void alterarCPU(int pid, int cpu) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    if (sched_setaffinity(pid, sizeof(mask), &mask) == -1) {
        perror("Erro ao alterar afinidade de CPU");
    } else {
        cout << "Processo " << pid << " movido para CPU " << cpu << "." << endl;
    }
}

void alterarPrioridade(int pid, int prioridade) {
    if (setpriority(PRIO_PROCESS, pid, prioridade) == -1) {
        perror("Erro ao alterar prioridade");
    } else {
        cout << "Prioridade do processo " << pid << " alterada para " << prioridade << "." << endl;
    }
}

int main() {
    int opcao, pid, cpu, prioridade;
    string filtro;

    while (true) {
        cout << "\nGerenciador de Tarefas - Menu de Opções:\n";
        cout << "1. Listar Processos\n";
        cout << "2. Matar Processo\n";
        cout << "3. Suspender Processo\n";
        cout << "4. Continuar Processo\n";
        cout << "5. Alterar CPU de Processo\n";
        cout << "6. Alterar Prioridade de Processo\n";
        cout << "0. Sair\n";
        cout << "Escolha uma opção: ";
        cin >> opcao;

        switch (opcao) {
            case 1:
                cout << "Digite um filtro (opcional): ";
                cin.ignore();
                getline(cin, filtro);
                listarProcessos(filtro);
                break;
            case 2:
                cout << "Digite o PID do processo a ser matado: ";
                cin >> pid;
                matarProcesso(pid);
                break;
            case 3:
                cout << "Digite o PID do processo a ser suspenso: ";
                cin >> pid;
                suspenderProcesso(pid);
                break;
            case 4:
                cout << "Digite o PID do processo a ser retomado: ";
                cin >> pid;
                continuarProcesso(pid);
                break;
            case 5:
                cout << "Digite o PID do processo: ";
                cin >> pid;
                cout << "Digite o número da CPU (0-7): ";
                cin >> cpu;
                alterarCPU(pid, cpu);
                break;
            case 6:
                cout << "Digite o PID do processo: ";
                cin >> pid;
                cout << "Digite a prioridade (-20 a 19): ";
                cin >> prioridade;
                alterarPrioridade(pid, prioridade);
                break;
            case 0:
                cout << "Saindo...\n";
                return 0;
            default:
                cout << "Opção inválida! Tente novamente.\n";
                break;
        }
    }

    return 0;
}
