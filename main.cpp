#include <iostream>
#include <string>
#include <cstdlib>
#include <signal.h>
#include <sched.h>
#include <sys/resource.h>
#include <unistd.h>
#include <cstring>
#include <iomanip>
#include <thread>
#include <mutex>
#include <atomic>

using namespace std;

mutex mtx;
atomic<bool> atualizando(true);
atomic<bool> filtroAtivo(false);
atomic<bool> comandoAtivo(false);
string filtroAtual;

void listarProcessos(const string &filtro) {
  string comando = "ps --no-header -eo comm,%cpu,etime,nlwp,psr,pid,user,pri,ni";


  // Incluir filtro, se houver
  if (!filtro.empty()) {
    comando += " | grep -i " + filtro;
  }

  // Limpar tela para exibir a lista atualizada
  system("clear");

  // Mostrar o cabeçalho da tabela
  cout << left << setw(20) << "Nome do Processo"
     << setw(7) << "% CPU"
     << setw(18) << "Tempo Atividade"
     << setw(9) << "Threads"
     << setw(9) << "CPU"
     << setw(9) << "PID"
     << setw(9) << "PRI"  
     << setw(9) << "NI"    
     << setw(20) << "Usuário" << endl;

  // Separação das colunas
  cout << string(95, '-') << endl;

  // Executar o comando para listar os processos
  FILE *fp = popen(comando.c_str(), "r");
  if (fp == nullptr) {
    perror("Erro ao executar comando");
    return;
  }

  // Exibir os dados de cada processo
  char nome[256], cpu[10], tempo[20], threads[10], cpuNum[10], pid[10], usuario[256], pri[10], ni[10];
while (fscanf(fp, "%s %s %s %s %s %s %s %s %s\n", nome, cpu, tempo, threads, cpuNum, pid, usuario, pri, ni) != EOF) {
    cout << setw(20) << nome
         << setw(10) << cpu
         << setw(15) << tempo
         << setw(10) << threads
         << setw(10) << cpuNum
         << setw(10) << pid
         << setw(10) << pri
         << setw(10) << ni
         << setw(20) << usuario << endl;
}

  fclose(fp);
}

void exibirLegenda() {
  // Legenda abaixo da tabela
  cout << "========================================\n";
  cout << "Menu:\n";
  cout << "0 - Filtrar processos.\n";
  cout << "1 - Matar um processo.\n";
  cout << "2 - Suspender um processo.\n";
  cout << "3 - Continuar um processo.\n";
  cout << "4 - Definir afinidade de CPU.\n";
  cout << "5 - Alterar a prioridade.\n";
  cout << "6 - Atualizar lista.\n";
  cout << "7 - Sair.\n";
  cout << "========================================\n";
}

void atualizarLista() {
  while (atualizando) {
    this_thread::sleep_for(chrono::seconds(1));

    if (!filtroAtivo && !comandoAtivo) {
      lock_guard<mutex> lock(mtx);
      listarProcessos(filtroAtual);
      exibirLegenda();
    }
  }
}

void matarProcesso(int pid) {
  comandoAtivo = true;
  if (kill(pid, SIGKILL) == -1) {
    perror("Erro ao matar o processo");
  } else {
    cout << "⚰ | Processo " << pid << " morto." << endl;
  }
  comandoAtivo = false;
}

void suspenderProcesso(int pid) {
  comandoAtivo = true;
  if (kill(pid, SIGSTOP) == -1) {
    perror("Erro ao suspender o processo");
  } else {
    cout << "Processo " << pid << " suspenso." << endl;
  }
  comandoAtivo = false;
}

void continuarProcesso(int pid) {
  comandoAtivo = true;
  if (kill(pid, SIGCONT) == -1) {
    perror("Erro ao continuar o processo");
  } else {
    cout << "Processo " << pid << " sobrevive..." << endl;
  }
  comandoAtivo = false;
}

void alterarCPU(int pid, int cpu) {
  comandoAtivo = true;
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);
  if (sched_setaffinity(pid, sizeof(mask), &mask) == -1) {
    perror("Erro ao alterar afinidade de CPU");
  } else {
    cout << "Processo " << pid << " movido para CPU " << cpu << "." << endl;
  }
  comandoAtivo = false;
}

void alterarPrioridade(int pid, int prioridade) {
  comandoAtivo = true;
  if (setpriority(PRIO_PROCESS, pid, prioridade) == -1) {
    perror("Erro ao alterar prioridade");
  } else {
    cout << "Prioridade do processo " << pid << " alterada para " << prioridade << "." << endl;
  }
  comandoAtivo = false;
}

int main() {
  int opcao, pid, cpu, prioridade;
  string filtro;

  thread atualizacaoThread(atualizarLista);

  exibirLegenda(); // Exibir a legenda no início

  while (true) {
    cout << "\nEscolha uma opção (0 a 7): ";
    cin >> opcao;
    cin.ignore(); // Limpar o buffer do teclado

    if (opcao < 0 || opcao > 7) {
      cout << "Opção inválida! Tente novamente.\n";
      continue;
    }

    switch (opcao) {
      case 0:
        filtroAtivo = true;
        cout << "--> Configurar filtro:\n";
        cout << "Digite o filtro: ";
        getline(cin, filtro);
        {
          lock_guard<mutex> lock(mtx);
          filtroAtual = filtro;
        }
        filtroAtivo = false;
        break;
      case 1:
        filtroAtivo = true;
        cout << "--> Configurar processo para matar:\n";
        cout << "Digite o PID para matar: ";
        cin >> pid;
        matarProcesso(pid);
        cin.ignore();
        break;
      case 2:
        filtroAtivo = true;
        cout << "--> Configurar processo para suspender:\n";
        cout << "Digite o PID para suspender: ";
        cin >> pid;
        suspenderProcesso(pid);
        filtroAtivo = false;
        cin.ignore();
        break;
      case 3:
        filtroAtivo = true;
        cout << "--> Configurar processo para continuar:\n";
        cout << "Digite o PID para continuar: ";
        cin >> pid;
        continuarProcesso(pid);
        filtroAtivo = false;
        cin.ignore();
        break;
      case 4:
        filtroAtivo = true;
        cout << "--> Configurar afinidade de CPU:\n";
        cout << "Digite o PID: ";
        cin >> pid;
        cout << "Digite a CPU: ";
        cin >> cpu;
        alterarCPU(pid, cpu);
        filtroAtivo = false;
        cin.ignore();
        break;
      case 5:
        filtroAtivo = true;
        cout << "--> Configurar prioridade de um processo:\n";
        cout << "Digite o PID: ";
        cin >> pid;
        cout << "Digite a prioridade: ";
        cin >> prioridade;
        alterarPrioridade(pid, prioridade);
        filtroAtivo = false;
        cin.ignore();
        break;
      case 6:
        break; // Atualizar lista automaticamente já está sendo feito pela thread
      case 7:
        atualizando = false;
        atualizacaoThread.join();
        return 0;
    }
  }
}
