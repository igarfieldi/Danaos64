#!/bin/bash

# Function to check if array contains value
function contains() {
	local n=$#
	local value=${!n}
		for ((i=1;i < $#;i++)) {
		if [ "${!i}" == "${value}" ]; then
			return "0"
		fi
	}
	return "1"
}

MAKEBINUTILS=0
MAKEGCC=0
MAKELIBGCC=1
CLEANUPPRE=0
CLEANUPSRC=0
CLEANUPBUILD=0

TARGETS=("i386-elf" "x86_64-elf")

# Default values for prefix and cores
PREFIX="/usr"
CORES=$(grep -c ^processor /proc/cpuinfo)

# Parse the options
while getopts ":p:c:h" OPTION; do
	case $OPTION in
		p) PREFIX=$OPTARG;;
		c) CORES=$OPTARG;;
		h) echo "Usage: -c NO_THREADS -p INSTALL_PREFIX [TARGETS ...]\nINSTALL_PREFIX defaults to \"/usr\", cores defaults to the number of processors available"; exit 0;;
		\?) echo "Invalid option: $OPTARG"; exit 1;;
		:) echo "Invalid option: $OPTARG requires an argument"; exit 1;;
	esac
done
# Shift to get the parsed options out of the list
shift $((OPTIND - 1))

# Check target validity
for TARGET in $@; do
	if ! contains ${TARGETS[@]} $TARGET; then
		echo "Unknown target: $TARGET"; exit 1
	fi
done

# Enable abort-on-error
set +e

# The the required archives
if [[ $MAKEBINUTILS == 1 ]]; then
	wget -nc http://ftp.gnu.org/gnu/binutils/binutils-2.29.tar.xz
	tar -xf binutils-2.29.tar.xz
	
	if [[ $CLEANUPPRE == 1 ]]; then
		rm -f binutils-2.29.tar.xz
	fi
fi
if [[ $MAKEGCC == 1 ]] || [[ $MAKELIBGCC == 1 ]]; then
	wget -nc http://ftp.gnu.org/gnu/gcc/gcc-7.2.0/gcc-7.2.0.tar.xz
	wget -nc http://ftp.gnu.org/gnu/gmp/gmp-6.1.2.tar.xz
	wget -nc http://ftp.gnu.org/gnu/mpfr/mpfr-3.1.6.tar.xz
	wget -nc http://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
	wget -nc http://isl.gforge.inria.fr/isl-0.18.tar.xz
	wget -nc https://www.bastoul.net/cloog/pages/download/cloog-0.18.4.tar.gz
	
	# Extract the archives
	tar -xf gcc-7.2.0.tar.xz
	tar -xf gmp-6.1.2.tar.xz
	tar -xf mpfr-3.1.6.tar.xz
	tar -xf mpc-1.0.3.tar.gz
	tar -xf isl-0.18.tar.xz
	tar -xf cloog-0.18.4.tar.gz
	
	if [[ $CLEANUPPRE == 1 ]]; then
		# Delete the now superfluous archives
		rm -f gcc-7.2.0.tar.xz
		rm -f gmp-6.1.2.tar.xz
		rm -f mpfr-3.1.6.tar.xz
		rm -f mpc-1.0.3.tar.gz
		rm -f isl-0.18.tar.xz
		rm -f cloog-0.18.4.tar.gz
	fi
	
	# Move GMP etc. into the GCC folder
	mv gmp-6.1.2 gcc-7.2.0/gmp
	mv mpfr-3.1.6 gcc-7.2.0/mpfr
	mv mpc-1.0.3 gcc-7.2.0/mpc
	mv isl-0.18 gcc-7.2.0/isl
	mv cloog-0.18.4 gcc-7.2.0/cloog
	
	# Patch GCC (remove the wrong warning about too small types when using bitfield enums)
	# This may need to be adapted for different GCC versions
	if grep -Fq "is too small to hold all values" gcc-7.2.0/gcc/cp/class.c; then
		sed -i -e '3586,3591d' gcc-7.2.0/gcc/cp/class.c
	fi

	# To use mcmodel=kernel and remove red-zone we need to modify libgcc's build
	# First add a new file announcing the multilib options:
	printf "MULTILIB_OPTIONS += mno-red-zone mcmodel=kernel\nMULTILIB_DIRNAMES += no-red-zone mcmodel-kernel" > gcc-7.2.0/gcc/config/i386/t-x86_64-elf

	# Then add it to the build files (adding the line multiple times does no harm since it's
	# a reassignment
	awk '/x86_64-\*-elf\*\)/ { print; print "	tmake_file=\"${tmake_file} i386/t-x86_64-elf\" # include the new multilib configuration"; next }1' gcc-7.2.0/gcc/config.gcc > tmp.txt
	mv tmp.txt gcc-7.2.0/gcc/config.gcc
fi


for TARGET in $@; do
	echo "Creating target $TARGET..."

	# Create the build directories
	if [[ $MAKEBINUTILS == 1 ]]; then
		# Binutils
		mkdir -p build-binutils-$TARGET

		cd build-binutils-$TARGET
		../binutils-2.29/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror --with-sysroot
		make -j $CORES
		make install
		cd ..
	fi

	if [[ $MAKEGCC == 1 ]] || [[ $MAKELIBGCC == 1 ]]; then
		# Configure GCC for the target
		mkdir -p build-gcc-$TARGET
		
		cd build-gcc-$TARGET
		../gcc-7.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --without-headers --enable-languages=c,c++
		cd ..
	fi
	
	if [[ $MAKEGCC == 1 ]]; then
		# GCC
		cd build-gcc-$TARGET
		make all-gcc -j $CORES
		cd ..
	fi
	
	if [[ $MAKELIBGCC == 1 ]]; then
		# Libgcc
		cd build-gcc-$TARGET
		
		# For x86-64 we need to patch the makefile to disable red zone and use mcmodel kernel
		if [[ "$TARGET" =~ "x86_64*" ]]; then
			make all-target-libgcc -j $CORES || true
			sed -i 's/PICFLAG/DISABLED_PICFLAG/g' $TARGET/no-red-zone/mcmodel-kernel/libgcc/Makefile
			sed -i 's/PICFLAG/DISABLED_PICFLAG/g' $TARGET/mcmodel-kernel/libgcc/Makefile
			make all-target-libgcc -j $CORES
		else
			make all-target-libgcc -j $CORES
		fi
		
		cd ..
	fi
	
	# Install the components
	
	if [[ $MAKEGCC == 1 ]]; then
		cd build-gcc-$TARGET
		make install-gcc
		cd ..
	fi
	if [[ $MAKELIBGCC == 1 ]]; then
		cd build-gcc-$TARGET
		make install-target-libgcc
		cd ..
	fi
	
	if [[ $CLEANUPBUILD == 1 ]]; then
		# Clean up build files
		rm -rf build-binutils-$TARGET
		rm -rf build-gcc-$TARGET
	fi
done

# Disable abort-on-error again
set -e

if [[ $CLEANUPSRC == 1 ]]; then
	# Clean up source files
	rm -rf gcc-7.2.0
	rm -rf binutils-2.29
fi
