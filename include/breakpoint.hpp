//
// Created by Madhav Ramesh on 12/18/23.
//

#ifndef BREAKPOINT_HPP
#define BREAKPOINT_HPP

#include <iostream>
#include <bits/ptrace-shared.h>
#include <sys/ptrace.h>

class Breakpoint {
    public:
        Breakpoint(pid_t pid, std::intptr_t addr)
        : m_pid(pid), m_addr(addr) {}

        bool is_enabled() const { return m_is_enabled; }

        std::intptr_t get_address() const { return m_addr; }

        void enable() {
            //peek at data
            auto data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
            //save first byte to m_saved_data
            m_saved_data = static_cast<uint8_t>(data & 0xFF);
            /*clear lower order byte,
              save int3 to lower order byte
              write the modified instruction back to the address
              */
            constexpr uint64_t int3 = 0xcc;
            const uint64_t modified_instr = ((data & ~0xFF) | int3);
            //pokedata and replace the first byte to 0xcc(int3)
            ptrace(PTRACE_POKEDATA, m_pid, m_addr, modified_instr);

            m_is_enabled = true;
        }

        void disable() {
            auto data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
            uint64_t instr_to_restore = ((data & ~0xFF) | m_saved_data);
            ptrace(PTRACE_POKEDATA, m_pid, m_addr, instr_to_restore);
            m_is_enabled = false;
        }

    private:
        pid_t m_pid;
        std::intptr_t m_addr;
        bool m_is_enabled;
        uint8_t m_saved_data;

};

#endif //BREAKPOINT_HPP
