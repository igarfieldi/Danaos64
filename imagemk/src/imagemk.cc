#include <iostream>
#include <fstream>
#include <stdint.h>

int main(int argc, const char **argv) {
    if(argc < 5) {
        std::cout << "Usage: " << argv[0] << " mbr bootloader kernel image [sector size]" << std::endl;
        return -1;
    }

    uint16_t sector_size = (argc < 6) ? 512 : static_cast<uint16_t>(std::stoul(argv[5]));

    std::ifstream mbr(argv[1], std::ifstream::binary | std::ifstream::in);
    std::ifstream kernel(argv[3], std::ifstream::binary | std::ifstream::ate | std::ifstream::in);
    uint16_t kernel_sectors = 1u + (static_cast<uint16_t>(kernel.tellg()) - 1u) / sector_size;

    std::cout << "Kernel sectors: " << kernel_sectors << std::endl;

    std::fstream bootloader(argv[2], std::fstream::binary | std::fstream::in | std::fstream::out);
    bootloader.seekp(504);

    const char *data = reinterpret_cast<char*>(&kernel_sectors);
    bootloader.write(data, sizeof(uint16_t));

    std::ofstream image(argv[4], std::ofstream::binary | std::fstream::out);
    image << mbr.rdbuf();
    bootloader.seekp(0);
    image << bootloader.rdbuf();
    kernel.seekg(0);
    image << kernel.rdbuf();

    return 0;
}