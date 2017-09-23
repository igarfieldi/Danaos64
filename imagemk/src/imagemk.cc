#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string>
#include <cstring>

int main(int argc, const char **argv) {
    if((argc < 2) || (std::strcmp(argv[1], "--help") == 0)) {
        std::cout << "Usage: " << argv[0] << "-b bootloader [size_pos_start size_pos_end] "
        			"-s sector_pos [-m mbr] [-k kernel [size_pos_start size_pos_end]] outfile" << std::endl;
        return -1;
    }

    size_t bootloader_size_pos_start = 1;
    size_t bootloader_size_pos_end = 0;
    size_t kernel_size_pos_start = 1;
    size_t kernel_size_pos_end = 0;

    std::string bootloader_name;
    std::string mbr_name;
    std::string kernel_name;
    std::string image_name;
    uint16_t sector_size = 0;

    // Parse the command line options
    {
    	size_t argindex = 1;
		for(; argindex < argc - 1; ++argindex) {
			// Check if the current "string" is one of the recognized options
			// Also, for each check if there are enough arguments left to satisfy the constraints
		    if(std::strcmp(argv[argindex], "-b") == 0) {
		    	if(argindex >= argc - 2) {
		    		std::cerr << "Wrong option format; [-b bootloader [size_pos_start size_pos_end]] required" << std::endl;
		    		return -1;
		    	}
		        bootloader_name = argv[++argindex];
		    	if(argindex < argc - 3) {
		    		bootloader_size_pos_start = std::stoull(argv[++argindex]);
		    		bootloader_size_pos_end = std::stoull(argv[++argindex]);
		    	}
		        
		    } else if(std::strcmp(argv[argindex], "-s") == 0) {
		    	if(argindex >= argc - 2) {
		    		std::cerr << "Wrong option format; [-s sector_size] required" << std::endl;
		    		return -1;
		    	}
		        sector_size = static_cast<uint16_t>(std::stoul(argv[++argindex]));
		    } else if(std::strcmp(argv[argindex], "-m") == 0) {
		    	if(argindex >= argc - 2) {
		    		std::cerr << "Wrong option format; [-m mbr] required" << std::endl;
		    		return -1;
		    	}
		        mbr_name = argv[++argindex];
		    } else if(std::strcmp(argv[argindex], "-k") == 0) {
		    	if(argindex >= argc - 2) {
		    		std::cerr << "Wrong option format; [-k kernel [size_pos_start size_pos_end]] required" << std::endl;
		    		return -1;
		    	}
		        kernel_name = argv[++argindex];
		    	if(argindex < argc - 3) {
		    		kernel_size_pos_start = std::stoull(argv[++argindex]);
		    		kernel_size_pos_end = std::stoull(argv[++argindex]);
		    	}
		    } else {
		        std::cerr << "Unrecognized option: " << argv[argindex] << std::endl;
		        return -1;
		    }
		}
		// Last string is always the output file
		image_name = argv[argc-1];
    }

	// Make sure we have a bootloader file given
    if(bootloader_name.empty()) {
        std::cerr << "Missing required option -b" << std::endl;
        return -1;
    }
    // Make sure we were given a sector size (no default)
    if((sector_size == 0) || (sector_size % 512 != 0)) {
        std::cerr << "Missing or invalid sector size (-s)" << std::endl;
        return -1;
    }

	// Check if the bootloader file is valid
    std::fstream bootloader(bootloader_name, std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::ate);
    if(!bootloader) {
        std::cerr << "Could not open bootloader file " << bootloader_name << std::endl;
        return -1;
    }
    // If we were given a valid position, write it into the bootloader file
    if(bootloader_size_pos_start < bootloader_size_pos_end) {
        size_t bootloader_sectors = 1u + (static_cast<size_t>(bootloader.tellg()) - 1u) / sector_size;
        std::cout << "Bootloader sectors: " << bootloader_sectors << std::endl;
        const char *data = reinterpret_cast<char*>(&bootloader_sectors);
        bootloader.seekp(bootloader_size_pos_start);
        bootloader.write(data, std::min(sizeof(size_t), bootloader_size_pos_end - bootloader_size_pos_start + 1));
    }

	// Check if we have a kernel we need to take care of
    if(!kernel_name.empty()) {   
        std::ifstream kernel(kernel_name, std::ifstream::binary | std::ifstream::ate | std::ifstream::in);
        if(!kernel) {
            std::cerr << "Could not open kernel file " << kernel_name << std::endl;
            return -1;
        }
		// If we were given a valid position, write it into the bootloader file
		if(kernel_size_pos_start < kernel_size_pos_end) {
		    size_t kernel_sectors = 1u + (static_cast<size_t>(kernel.tellg()) - 1u) / sector_size;
		    std::cout << "Kernel sectors: " << kernel_sectors << std::endl;
		    const char *data = reinterpret_cast<char*>(&kernel_sectors);
		    bootloader.seekp(kernel_size_pos_start);
		    bootloader.write(data, std::min(sizeof(size_t), kernel_size_pos_end - kernel_size_pos_start + 1));
		}
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
