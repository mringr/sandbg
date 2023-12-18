//
// Created by Madhav Ramesh on 12/17/23.
//

#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <iostream>
#include <string>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <linenoise.h>

#include "helpers.hpp"

class Debugger {
    public:
        Debugger(std::string program_name, pid_t pid)
        : m_program_name(std::move(program_name)), m_pid(pid) {}

        void run() {
            int wait_status;
            constexpr auto options = 0;
            waitpid(m_pid, &wait_status, options);

            char* line = nullptr;
            while((line = linenoise("sandbg> ")) != nullptr ) {
                handle_command(line);
                linenoiseHistoryAdd(line);
                linenoiseFree(line);
            }
        }

    private:
        std::string m_program_name;
        pid_t m_pid;

        void handle_command(const std::string& line) {
            auto args = Helpers::split(line, ' ');
            auto command = args[0];

            if(Helpers::is_prefix(command, "continue")) {
                continue_execution();
            }
            else {
                std::cerr << "Unknown command\n" ;
            }
        }

        void continue_execution() {
            ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
            int wait_status;
            auto options = 0;

            waitpid(m_pid, &wait_status, options);
        }
};
#endif //DEBUGGER_HPP
