#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>

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
            ptrace(PTRACE_TRACEME, pid, nullptr, nullptr);
            execl(prog, prog, nullptr);

        default:
            std::cout << "In the parent process. Child pid = " << pid << "\n";
    }
    return 0;
}
