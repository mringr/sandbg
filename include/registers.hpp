//
// Created by Madhav Ramesh on 12/19/23.
//

#ifndef REGISTERS_HPP
#define REGISTERS_HPP
#include <array>
#include <cstddef>
#include <string>

namespace sandbg {
    /* maintaining the same register order as in user.h */
    enum class reg {
        r15, r14, r13, r12,
        rbp, rbx, r11, r10,
        r9, r8, rax, rcx,
        rdx, rsi, rdi, orig_rax,
        rip, cs, eflags, rsp,
        ss, fs_base, gs_base, ds,
        es, fs, gs
    };

    constexpr std::size_t n_registers = 27;

    struct reg_descriptor {
        reg r;
        int dwarf_r;
        std::string name;
    };

    /* global register descriptor array of 27 elements */
    const std::array<reg_descriptor, n_registers> g_register_descriptors {{
            {reg::r15, 15, "r15"},
            { reg::r14, 14, "r14" },
            { reg::r13, 13, "r13" },
            { reg::r12, 12, "r12" },
            { reg::rbp, 6, "rbp" },
            { reg::rbx, 3, "rbx" },
            { reg::r11, 11, "r11" },
            { reg::r10, 10, "r10" },
            { reg::r9, 9, "r9" },
            { reg::r8, 8, "r8" },
            { reg::rax, 0, "rax" },
            { reg::rcx, 2, "rcx" },
            { reg::rdx, 1, "rdx" },
            { reg::rsi, 4, "rsi" },
            { reg::rdi, 5, "rdi" },
            { reg::orig_rax, -1, "orig_rax" },
            { reg::rip, -1, "rip" },
            { reg::cs, 51, "cs" },
            { reg::eflags, 49, "eflags" },
            { reg::rsp, 7, "rsp" },
            { reg::ss, 52, "ss" },
            { reg::fs_base, 58, "fs_base" },
            { reg::gs_base, 59, "gs_base" },
            { reg::ds, 53, "ds" },
            { reg::es, 50, "es" },
            { reg::fs, 54, "fs" },
            { reg::gs, 55, "gs" },
        }
    };
}

#endif //REGISTERS_HPP
