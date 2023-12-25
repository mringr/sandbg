//
// Created by Madhav Ramesh on 12/24/23.
//
#include <cinttypes>
#include <iostream>

#include <elf++.hh>
#include <dwarf++.hh>
#include <fcntl.h>

void
dump_line_table(const dwarf::line_table &lt)
{
    for (auto &line : lt) {
        if (line.end_sequence)
            printf("\n");
        else
            printf("%-40s%8d%#20" PRIx64 "\n", line.file->path.c_str(),
                   line.line, line.address);
    }
}

int main (int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << "<elf_file>\n";
        return 2;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        std::cerr << argv[1] << " : " << strerror(errno) << "\n";
    }

    elf::elf _elf {elf::create_mmap_loader(fd)};
    dwarf::dwarf _dwarf {dwarf::elf::create_loader(_elf)};


    for(auto& cu : _dwarf.compilation_units()) {
        //std::cout << "--- <" << std::hex << (unsigned int)cu.get_section_offset() << ">\n";
        printf("--- <%x>\n", (unsigned int)cu.get_section_offset());
        cu.get_line_table();
        dump_line_table(cu.get_line_table());
        printf("\n");
    }

    return 0;
}
