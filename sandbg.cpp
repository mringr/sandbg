#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/personality.h>

#include "include/debugger.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "Program name not specified";
        return -1;
    }

    auto prog = argv[1];
    auto pid = fork();

    switch (pid) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);

        case 0:
            personality(ADDR_NO_RANDOMIZE);
            ptrace(PTRACE_TRACEME, pid, nullptr, nullptr);
            execl(prog, prog, nullptr);
            std::cerr << "Exec returned error\n";
            exit(EXIT_FAILURE);

        default:
            std::cout << "In the parent process. Child pid = " << pid << "\n";
            Debugger dbg {prog, pid};
            dbg.run();
    }
    return 0;
}
