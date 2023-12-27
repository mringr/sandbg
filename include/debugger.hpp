//
// Created by Madhav Ramesh on 12/17/23.
//

#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

#include <dwarf++.hh>
#include <elf++.hh>
#include <linenoise.h>

#include "breakpoint.hpp"

#include "helpers.hpp"
#include "registers.hpp"

class Debugger {
    public:
        Debugger(std::string program_name, pid_t pid)
        : m_program_name(std::move(program_name)), m_pid(pid) {
            auto fd = open(m_program_name.c_str(), O_RDONLY);

            if (fd < 0) {
                std::cerr << "Invalid program name\n";
                throw std::invalid_argument("Invalid program name");
            }

            m_elf = elf::elf{elf::create_mmap_loader(fd)};
            m_dwarf = dwarf::dwarf{dwarf::elf::create_loader(m_elf)};
        }

        void run() {
            wait_for_signal();
            initialize_load_address();

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
        uint64_t m_load_address;

        dwarf::dwarf m_dwarf;
        elf::elf m_elf;

        void initialize_load_address() {
            if (m_elf.get_hdr().type == elf::et::dyn) {
                std::ifstream map("/proc/" + std::to_string(m_pid) + "/maps");
                std::string addr;
                std::getline(map, addr, '-');

                m_load_address = std::stoi(addr, 0, 16);
            }
        }

        uint64_t offset_load_address(const uint64_t addr) const {
            return addr - m_load_address;
        }

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

            auto siginfo = get_signal_info();
            switch (siginfo.si_signo) {
                case SIGTRAP:
                    handle_sigtrap(siginfo);
                    break;
                case SIGSEGV:
                    std::cerr << "Segfault reason: " << siginfo.si_code << "\n";
                    break;
                default:
                    std::cerr << "Signal: " << siginfo.si_code << "\n";
            }
        }

        void handle_sigtrap(const siginfo_t siginfo) const {
            switch (siginfo.si_code) {
                case TRAP_BRKPT:
                case SI_KERNEL: {
                    set_pc(get_pc() - 1);
                    std::cout << "Hit breakpoint at " << std::hex << get_pc() << "\n";
                    auto offset_pc = offset_load_address(get_pc());
                    auto line_entry = get_line_entry_from_pc(get_pc());
                    print_source(line_entry->file->path, line_entry->line);
                    return;

                }
                //TRAP_TRACE sent during single stepping
                case TRAP_TRACE:
                    return;
                default:
                    std::cerr << "Unknow sigtrap code: " << siginfo.si_code << "\n";
            }
        }

        void step_over_breakpoint() {


            /* check if instruction is a breakpoint. NO OP otherwise */
            if (m_breakpoints.contains(get_pc())) {
                set_pc(get_pc());
                Breakpoint& bp = m_breakpoints[get_pc()];
                bp.disable();
                ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
                wait_for_signal();
                bp.enable();
            }
        }

        dwarf::die get_function_from_pc(uint64_t pc) {
            for (auto& cu : m_dwarf.compilation_units()) {
                dwarf::die cu_root = cu.root();
                dwarf::rangelist range = dwarf::die_pc_range(cu_root);
                if (range.contains(pc)) {
                    for (auto& die : cu_root) {
                        if (die.tag == dwarf::DW_TAG::subprogram
                            && dwarf::die_pc_range(die).contains(pc)) {
                            return die;
                        }
                    }
                }
            }
            throw std::out_of_range("Cannot find function");
        }

        dwarf::line_table::iterator get_line_entry_from_pc (uint64_t pc) const {
            for (auto& cu : m_dwarf.compilation_units()) {
                if (dwarf::die_pc_range(cu.root()).contains(pc)) {
                    const dwarf::line_table& lt = cu.get_line_table();
                    const auto it = lt.find_address(pc);
                    if(it == lt.end()) {
                        throw std::out_of_range("Unable to find line entry for address");
                    }
                    return it;
                }
            }
            throw std::out_of_range("Unable to find line entry for address - OOL");
        }

        siginfo_t get_signal_info() const {
            siginfo_t siginfo;
            ptrace(PTRACE_GETSIGINFO, m_pid, nullptr, &siginfo);
            return siginfo;
        }

        /* TODO: Using the same original foo from TartalLlama, can this be improved? */
        void print_source (const std::string& file_name, unsigned line, unsigned n_lines_context=2) const {
            std::ifstream file {file_name};
            /* n_lines_context is # of adj. lines */
            auto start_line = (line <= n_lines_context) ? 1 : line - n_lines_context;
            auto end_line = line + n_lines_context + (line < n_lines_context ? n_lines_context - line : 0) + 1;

            char c{};
            auto current_line = 1u;

            //Skip lines up until start_line
            while (current_line != start_line && file.get(c)) {
                if (c == '\n') {
                    ++current_line;
                }
            }

            //Output cursor if we're at the current line
            std::cout << (current_line==line ? "> " : "  ");

            //Write lines up until end_line
            while (current_line <= end_line && file.get(c)) {
                std::cout << c;
                if (c == '\n') {
                    ++current_line;
                    //Output cursor if we're at the current line
                    std::cout << (current_line==line ? "> " : "  ");
                }
            }

            //Write newline and make sure that the stream is flushed properly
            std::cout << std::endl;

        }
};
#endif //DEBUGGER_HPP
