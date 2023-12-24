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
#include <unordered_map>

#include "breakpoint.hpp"

#include "helpers.hpp"
#include "registers.hpp"

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

        void set_breakpoint_at_address(std::intptr_t addr) {
            std::cout << "Set breakpoint at addr 0x: " << std::hex << addr << "\n";
            Breakpoint breakpoint {m_pid, addr};
            breakpoint.enable();
            m_breakpoints[addr] = breakpoint;
        }

        uint64_t read_memory (uint64_t addr) const {
            return ptrace(PTRACE_PEEKDATA, m_pid, addr, nullptr);
        }

        void write_memory(uint64_t addr, uint64_t data) const {
            ptrace(PTRACE_POKEDATA, m_pid, addr, data);
        }

    private:
        std::string m_program_name;
        pid_t m_pid;
        std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;

        void handle_command(const std::string& line) {
            auto args = Helpers::split(line, ' ');
            auto command = args[0];

            if (Helpers::is_prefix(command, "continue")) {
                continue_execution();
            }
            else if (Helpers::is_prefix(command, "break")) {
                //TODO: Validation of address needs improvement
                if(args[1][0] == '0' && args[1][1] == 'x') {
                    std::string addr {args[1], 2};
                    set_breakpoint_at_address(std::stol(addr, nullptr, 16));
                }
            }
            else if (Helpers::is_prefix(command, "register")) {
                if (Helpers::is_prefix(args[1], "dump")) {
                    sandbg::dump_registers(m_pid);
                }
                else if (Helpers::is_prefix(args[1], "read")) {
                    std::cout << sandbg::get_register_value(m_pid, sandbg::get_register_from_name(args[2])) << "\n";
                }
                else if (Helpers::is_prefix(args[1], "write")) {
                    std::string val {args[3], 2};
                    sandbg::set_register_value(m_pid, sandbg::get_register_from_name(args[2]), std::stol(val, 0, 16));
                }
            }
            else if (Helpers::is_prefix(command, "memory")) {
                std::string addr { args[2], 2};

                if (Helpers::is_prefix(args[1], "read")) {
                    std::cout << read_memory(std::stol(addr, 0, 16)) << "\n";
                }
                else if (Helpers::is_prefix(args[1], "write")) {
                    std::string val{args[3], 2};
                    write_memory(std::stol(addr, 0, 16), std::stol(args[3], 0, 16));
                }
            }
            else {
                std::cerr << "Unknown command\n" ;
            }
        }

        void continue_execution() {
            step_over_breakpoint();
            ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
            wait_for_signal();
        }

        std::intptr_t get_pc() const {
            return static_cast<std::intptr_t>(get_register_value(m_pid, sandbg::reg::rip));
        }

        void set_pc(const uint64_t pc) const {
            sandbg::set_register_value(m_pid, sandbg::reg::rip, pc);
        }

        void wait_for_signal() const {
            int wait_status;
            constexpr auto options = 0;
            waitpid(m_pid, &wait_status, options);
        }

        void step_over_breakpoint() {
            /* Since PC will increment after execution of an instruction
             * for a breakpoint, the instr will be a 1 byte int3 instruction
             */
            auto previous_instr_addr = get_pc() - 1;

            /* check if instruction is a breakpoint. NO OP otherwise */
            if (m_breakpoints.contains(previous_instr_addr)) {
                set_pc(previous_instr_addr);
                Breakpoint& bp = m_breakpoints[previous_instr_addr];
                bp.disable();
                ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
                wait_for_signal();
                bp.enable();
            }

        }
};
#endif //DEBUGGER_HPP
