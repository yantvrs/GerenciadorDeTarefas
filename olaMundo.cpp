#include <iostream>
#include <thread>
#include <chrono>

void helloWorldProcess() {
    while (true) {
        std::cout << "Hello, World!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Aguardar 1 segundo
    }
}

int main() {
    try {
        helloWorldProcess();
    } catch (...) {
        std::cout << "\nProcesso encerrado." << std::endl;
    }
    return 0;
}
