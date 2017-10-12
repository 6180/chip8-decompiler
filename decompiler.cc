
#include <fstream>
#include <exception>

#include <cstdint>
#include <cstdlib>
#include <cstdio>

enum InsnType {
    kInvalid = 0,
    kReg,
    kJump,
    kCall,
    kRet
};

struct Chip8Insn {
    uint16_t opcode;
    InsnType type;
};

struct Insn_SYS : Chip8Insn {
    std::string_view mnemonic{"SYS 0x%06d"};
    uint8_t opcode;
    uint8_t addr;

    Insn_SYS(uint8_t opcode) : opcode(opcode), addr(opcode & 0x0FFF) {};
};

struct Chip8Rom {
public:
    Chip8Rom(const char *fname);
    ~Chip8Rom();

    char *rom;
    size_t rom_size;
};

Chip8Rom::Chip8Rom(const char *fname) {
    std::fstream fp{fname, std::ios::in|std::ios::binary|std::ios::ate};

    if (!fp) {
        throw std::runtime_error("[ERROR] could not open rom image.");
    }

    rom_size = fp.tellg();
    fp.seekg(std::ios_base::beg);

    rom = (rom_size < 0xE00) ? (char *)malloc(0x1000) : (char *)malloc(0x200 + rom_size);

    if (rom == nullptr) {
        throw std::runtime_error("[ERROR] allocating memory");
    }

    fp.read(rom+512, rom_size);

    if (fp.fail()) {
        throw std::runtime_error("[ERROR] reading from file");
    }
}

Chip8Rom::~Chip8Rom() {
    if (rom != nullptr) {
        free(rom);
    }
}

void usage() {
    puts("usage: c8dc filename");
}

int main(int argc, char **argv) {
    if (argc != 2){
        usage();
        return EXIT_SUCCESS;
    }

    Chip8Rom *rom;

    try {
        rom = new Chip8Rom(argv[1]);
    } catch(std::runtime_error e) {
        puts(e.what());
    }

    for(size_t i = 0; i < 120; ++i) {
        unsigned short addr = 0x200+i*2;
        uint16_t insn = rom->rom[addr] * 0x100 + rom->rom[addr+1];

        unsigned int u = insn >> 12 & 0xF;
        unsigned int l = insn & 0xF;

        unsigned int nnn = insn & 0xFFF;
        unsigned int n = insn & 0xF;
        unsigned int x = insn >> 8 & 0xF;
        unsigned int y = insn >> 4 & 0xF;
        unsigned int kk = insn & 0xFF;

        if (u == 0x0) {
            printf("%04X\n", insn);
        } else if (u == 0x1) {
            printf("\t0x%04X:\t%04x\tjmp  0x%04x\n", addr, insn, nnn);
        } else if (u == 0x2) {
            printf("\t0x%04X:\t%04x\tcall 0x%04x\n", addr, insn, nnn);
        } else if (u == 0x3) {
            printf("\t0x%04X:\t%04x\tse   v%d, 0x%02x (%d)\n", addr, insn, x, kk, kk);
        } else if (u == 0x4) {
            printf("\t0x%04X:\t%04x\tsne  v%d, 0x%02x (%d)\n", addr, insn, x, kk, kk);
        } else if (u == 0x5) {
            printf("\t0x%04X:\t%04x\tse   v%d, v%d (%d)\n", insn, x, y);
        } else if (u == 0x6) {
            printf("\t0x%04X:\t%04x\tld   v%d, 0x%02x (%d)\n", addr, insn, x, kk, kk);
        } else if (u == 0x7) {
            printf("\t0x%04X:\t%04x\tadd  v%d, 0x%02x (%d)\n", addr, insn, x, kk, kk);
        } else if (u == 0x8) {
            if (l == 0x0) {
                printf("\t0x%04X:\t%04x\tld   v%d, v%d\n", addr, insn, x, y);
            } else if (l == 0x1) {
                printf("\t0x%04X:\t%04x\tor   v%d, v%d\n", addr, insn, x, y);
            } else if (l == 0x2) {
                printf("\t0x%04X:\t%04x\tand  v%d, v%d\n", addr, insn, x, y);
            } else if (l == 0x3) {
                printf("\t0x%04X:\t%04x\txor  v%d, v%d\n", addr, insn, x, y);
            } else if (l == 0x4) {
                printf("\t0x%04X:\t%04x\tadd  v%d, v%d\n", addr, insn, x, y);
            } else if (l == 0x5) {
                printf("\t0x%04X:\t%04x\tsub  v%d, v%d\n", addr, insn, x, y);
            } else if (l == 0x6) {
                printf("\t0x%04X:\t%04x\tshr  v%d\n", addr, insn, x);
            } else if (l == 0x7) {
                printf("\t0x%04X:\t%04x\tsubn v%d, v%d\n", addr, insn, x, y);
            } else if (l == 0xE) {
                printf("\t0x%04X:\t%04x\tshl  v%d\n", addr, insn, x);
            }
        } else if (u == 0x9) {
            printf("\t0x%04X:\t%04x\tsne  v%d, v%d (%d)\n", insn, x, y);
        } else if (u == 0xA) {
            printf("\t0x%04X:\t%04x\tld   I, 0x%04x (%d)\n", addr, insn, nnn, nnn);
        } else if (u == 0xB) {
            printf("\t0x%04X:\t%04x\tjp   v0, 0x%04x (%d)\n", insn, nnn);
        } else if (u == 0xC) {
            printf("\t0x%04X:\t%04x\trnd  v%d, 0x%02x (%d)\n", addr, insn, x, kk, kk);
        } else if (u == 0xD) {
            printf("\t0x%04X:\t%04x\tdrw  v%d, v%d, 0x%x (%d)\n", addr, insn, x, y, n, n);
        } else if (u == 0xE) {
            if (l == 0x9E) {
                printf("\t0x%04X:\t%04x\tskp  v%d\n", addr, insn, x);
            } else if (l == 0xA1) {
                printf("\t0x%04X:\t%04x\tsknp v%d\n", addr, insn, x);
            }
        } else if (u == 0xF) {
            if (l == 0x07) {
                printf("\t0x%04X:\t%04x\tld   v%d, DT\n", addr, insn, x);
            } else if (l == 0x0A) {
                printf("\t0x%04X:\t%04x\tld   v%d, K\n", addr, insn, x);
            } else if (l == 0x15) {
                printf("\t0x%04X:\t%04x\tld   DT, v%d\n", addr, insn, x);
            } else if (l == 0x18) {
                printf("\t0x%04X:\t%04x\tld   ST, v%d\n", addr, insn, x);
            } else if (l == 0x1E) {
                printf("\t0x%04X:\t%04x\tadd I, v%d\n", addr, insn, x);
            } else if (l == 0x29) {
                printf("\t0x%04X:\t%04x\tld   F, v%d\n", addr, insn, x);
            } else if (l == 0x33) {
                printf("\t0x%04X:\t%04x\tld   B, v%d\n", addr, insn, x);
            } else if (l == 0x55) {
                printf("\t0x%04X:\t%04x\tld   [I], v%d\n", addr, insn, x);
            } else if (l == 0x65) {
                printf("\t0x%04X:\t%04x\tld   v%d, [I]\n", addr, insn, x);
            }
        }

    delete rom;

    return EXIT_SUCCESS;
}


