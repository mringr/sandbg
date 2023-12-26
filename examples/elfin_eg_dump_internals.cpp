//
// Created by Madhav Ramesh on 12/26/23.
//

#include <cinttypes>
#include <cstring>
#include <dwarf++.hh>
#include <elf++.hh>
#include <fcntl.h>
#include <iostream>

void
dump_tree(const dwarf::die &node, int depth = 0)
{
    printf("%*.s<%" PRIx64 "> %s\n", depth, "",
           node.get_section_offset(),
           to_string(node.tag).c_str());
    for (auto &attr : node.attributes())
        printf("%*.s      %s %s\n", depth, "",
               to_string(attr.first).c_str(),
               to_string(attr.second).c_str());
    for (auto &child : node)
        dump_tree(child, depth + 1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << "<elf_file>\n";
        return 2;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        std::cerr << argv[1] << " : " << strerror(errno) << "\n";
        return 1;
    }

    elf::elf _elf {elf::create_mmap_loader(fd)};

    std::cout << "\n====:Dump Sections:====\n";
    printf("  [Nr] %-16s %-16s %-16s %s\n",
               "Name", "Type", "Address", "Offset");
    printf("       %-16s %-16s %-15s %5s %4s %5s\n",
           "Size", "EntSize", "Flags", "Link", "Info", "Align");


    int i = 0;

    for(auto& sec : _elf.sections()) {
        auto &hdr = sec.get_hdr();
        printf("  [%2d] %-16s %-16s %016" PRIx64 " %08" PRIx64 "\n", i++,
                       sec.get_name().c_str(),
                       to_string(hdr.type).c_str(),
                       hdr.addr, hdr.offset);
        printf("       %016zx %016" PRIx64 " %-15s %5s %4d %5" PRIu64 "\n",
               sec.size(), hdr.entsize,
               to_string(hdr.flags).c_str(), to_string(hdr.link).c_str(),
               (int)hdr.info, hdr.addralign);
    }

    std::cout << "\n";


    std::cout << "\n====:Dump Segments:====\n";
    printf("  %-16s  %-16s   %-16s   %s\n",
                "Type", "Offset", "VirtAddr", "PhysAddr");
    printf("  %-16s  %-16s   %-16s  %6s %5s\n",
            " ", "FileSiz", "MemSiz", "Flags", "Align");
    for (auto &seg : _elf.segments()) {
        auto &hdr = seg.get_hdr();
        printf("   %-16s 0x%016" PRIx64 " 0x%016" PRIx64 " 0x%016" PRIx64 "\n",
               elf::to_string(hdr.type).c_str(), hdr.offset,
                hdr.vaddr, hdr.paddr);
        printf("   %-16s 0x%016" PRIx64 " 0x%016" PRIx64 " %-5s %-5" PRIx64 "\n", "",
                hdr.filesz, hdr.memsz,
                elf::to_string(hdr.flags).c_str(), hdr.align);
    }
    std::cout << "\n";


    std::cout << "\n====Dump Symbols:====\n";
    for (auto &sec : _elf.sections()) {
        if (sec.get_hdr().type != elf::sht::symtab && sec.get_hdr().type != elf::sht::dynsym)
            continue;

        printf("Symbol table '%s':\n", sec.get_name().c_str());
        printf("%6s: %-16s %-5s %-7s %-7s %-5s %s\n",
               "Num", "Value", "Size", "Type", "Binding", "Index",
               "Name");
        int i = 0;
        for (auto sym: sec.as_symtab()) {
            auto& d = sym.get_data();
            printf("%6d: %016" PRIx64 " %5" PRId64 " %-7s %-7s %5hd %s\n",
                   i++, d.value, d.size,
                   elf::to_string(d.type()).c_str(),
                   elf::to_string(d.binding()).c_str(),
                   d.shnxd,
                   sym.get_name().c_str());
        }
    }
    std::cout << "\n";

    std::cout << "\n====Dump Tree:====\n";
    dwarf::dwarf dw(dwarf::elf::create_loader(_elf));

    for (const auto& cu : dw.compilation_units()) {
        printf("--- <%" PRIx64 ">\n", cu.get_section_offset());
        dump_tree(cu.root());
    }
    std::cout << "\n";
}