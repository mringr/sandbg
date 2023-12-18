//
// Created by Madhav Ramesh on 12/17/23.
//

#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <string>
#include <sys/wait.h>

#include <linenoise.h>

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
                //handle_command(line)
                linenoiseHistoryAdd(line);
                linenoiseFree(line);
            }
        }

    private:
        std::string m_program_name;
        pid_t m_pid;
};
#endif //DEBUGGER_HPP
