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

TARGETS=("i386-elf" "x86_64-elf")

# Request sudo
if [ $EUID != 0 ]; then
	sudo "$0" "$@"
	exit $?
fi

# Default values for prefix and cores
PREFIX="/usr"
CORES=$(grep -c ^processor /proc/cpuinfo)

# Parse the options
while getopts ":p:c:h" OPTION; do
	case $OPTION in
		p) PREFIX=$OPTARG;;
		c) CORES=$OPTARG;;
		h) echo "Usage: -c NO_THREADS -p INSTALL_PREFIX [TARGETS ...]"; exit 0;;
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
wget -nc http://ftp.gnu.org/gnu/binutils/binutils-2.29.tar.xz
wget -nc http://ftp.gnu.org/gnu/gcc/gcc-7.2.0/gcc-7.2.0.tar.xz
wget -nc http://ftp.gnu.org/gnu/gmp/gmp-6.1.2.tar.xz
wget -nc http://ftp.gnu.org/gnu/mpfr/mpfr-3.1.6.tar.xz
wget -nc http://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
wget -nc http://isl.gforge.inria.fr/isl-0.18.tar.xz
wget -nc https://www.bastoul.net/cloog/pages/download/cloog-0.18.4.tar.gz

# Extract the archives
tar -xf binutils-2.29.tar.xz
tar -xf gcc-7.2.0.tar.xz
tar -xf gmp-6.1.2.tar.xz
tar -xf mpfr-3.1.6.tar.xz
tar -xf mpc-1.0.3.tar.gz
tar -xf isl-0.18.tar.xz
tar -xf cloog-0.18.4.tar.gz

# Delete the now superfluous archives
rm -f binutils-2.29.tar.xz
rm -f gcc-7.2.0.tar.xz
rm -f gmp-6.1.2.tar.xz
rm -f mpfr-3.1.6.tar.xz
rm -f mpc-1.0.3.tar.gz
rm -f isl-0.18.tar.xz
rm -f cloog-0.18.4.tar.gz

# Move GMP etc. into the GCC folder
mv gmp-6.1.2 gcc-7.2.0/gmp
mv mpfr-3.1.6 gcc-7.2.0/mpfr
mv mpc-1.0.3 gcc-7.2.0/mpc
mv isl-0.18 gcc-7.2.0/isl
mv cloog-0.18.4 gcc-7.2.0/cloog

# Patch GCC (remove the wrong warning about too small types when using bitfield enums)
sed -i -e '3586,3591d' gcc-7.2.0/gcc/cp/class.c

for TARGET in $@; do
	echo "Creating target $(TARGET)..."

	# Create the build directories
	mkdir build-binutils-$TARGET
	mkdir build-gcc-$TARGET

	# Binutils for i386-elf
	cd build-binutils-$TARGET
	../binutils-2.29/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror --with-sysroot
	make -j 4
	sudo make install

	# GCC for i386-elf
	cd ../build-gcc-$TARGET
	../gcc-7.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --without-headers --enable-languages=c,c++
	make all-gcc -j $CORES
	make all-target-libgcc -j $CORES
	sudo make install-gcc
	sudo make install-target-libgcc
	
	# Clean up a bit
	cd ..
	rm -rf build-binutils-$TARGET
	rm -rf build-gcc-$TARGET
done

# Disable abort-on-error again
set -e

# Final clean up
rm -rf gcc-7.2.0
rm -rf binutils-2.29

