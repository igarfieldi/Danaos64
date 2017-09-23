#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string>
#include <cstring>

int main(int argc, const char **argv) {
    if((argc < 2) || (std::strcmp(argv[1], "--help") == 0)) {
        std::cout << "Usage: " << argv[0] << "-b bootloader -s sector_size [-m mbr] [-k kernel] outfile" << std::endl;
        return -1;
    }

    constexpr size_t bootloader_size_pos = 502;
    constexpr size_t kernel_size_pos = 504;

    std::string bootloader_name;
    std::string mbr_name;
    std::string kernel_name;
    std::string image_name;
    uint16_t sector_size = 0;

    // TODO: there ought to be better ways to parse cmd options
    for(size_t argindex = 1; argindex < argc - 1; ++argindex) {
        if(std::strcmp(argv[argindex], "-b") == 0) {
            bootloader_name = argv[++argindex];
        } else if(std::strcmp(argv[argindex], "-s") == 0) {
            sector_size = static_cast<uint16_t>(std::stoul(argv[++argindex]));
        } else if(std::strcmp(argv[argindex], "-m") == 0) {
            mbr_name = argv[++argindex];
        } else if(std::strcmp(argv[argindex], "-k") == 0) {
            kernel_name = argv[++argindex];
        } else {
            std::cerr << "Unrecognized option: " << argv[argindex] << std::endl;
            return -1;
        }
    }
    image_name = argv[argc-1];

    if(bootloader_name.empty()) {
        std::cerr << "Missing required option -b" << std::endl;
        return -1;
    }
    if((sector_size == 0) || (sector_size % 512 != 0)) {
        std::cerr << "Missing or invalid sector size (-s)" << std::endl;
        return -1;
    }

    std::fstream bootloader(bootloader_name, std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::ate);
    if(!bootloader) {
        std::cerr << "Could not open bootloader file " << bootloader_name << std::endl;
        return -1;
    }
    {
        uint16_t bootloader_sectors = 1u + (static_cast<uint16_t>(bootloader.tellg()) - 1u) / sector_size;
        std::cout << "Bootloader sectors: " << bootloader_sectors << std::endl;
        const char *data = reinterpret_cast<char*>(&bootloader_sectors);
        bootloader.seekp(bootloader_size_pos);
        bootloader.write(data, sizeof(uint16_t));
    }

    if(!kernel_name.empty()) {   
        std::ifstream kernel(kernel_name, std::ifstream::binary | std::ifstream::ate | std::ifstream::in);
        if(!kernel) {
            std::cerr << "Could not open kernel file " << kernel_name << std::endl;
            return -1;
        }
        uint16_t kernel_sectors = 1u + (static_cast<uint16_t>(kernel.tellg()) - 1u) / sector_size;
        std::cout << "Kernel sectors: " << kernel_sectors << std::endl;
        const char *data = reinterpret_cast<char*>(&kernel_sectors);
        bootloader.seekp(kernel_size_pos);
        bootloader.write(data, sizeof(uint16_t));
    }

    std::ofstream image(image_name, std::ofstream::binary | std::fstream::out);
    if(!image) {
        std::cerr << "Could not create image file " << image_name << std::endl;
        return -1;
    }

    if(!mbr_name.empty()) {
        std::ifstream mbr(mbr_name, std::ifstream::binary | std::ifstream::in);
        if(!mbr) {
            std::cerr << "Could not open MBR file " << mbr_name << std::endl;
            return -1;
        }
        image << mbr.rdbuf();
    }

    bootloader.seekp(0);
    image << bootloader.rdbuf();

    if(!kernel_name.empty()) {   
        std::ifstream kernel(kernel_name, std::ifstream::binary | std::ifstream::in);
        image << kernel.rdbuf();
    }

    return 0;
}