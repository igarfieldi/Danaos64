/**
 * machine/io/port.h
**/

#ifndef DANAOS_MACHINE_IO_PORT_H_
#define DANAOS_MACHINE_IO_PORT_H_

#include <stddef.h>
#include <stdint.h>

// TODO: what about IO that needs a short delay?
// TODO: avoid the code duplication
// TODO: make this more elegant!

namespace hal {

	template <class T, size_t BYTES = sizeof(T)>
	class in_port;

	template <class T, size_t BYTES = sizeof(T)>
	class out_port;

	template <class T>
	class in_port<T, 1> {
	private:
		const uint16_t ADDRESS;

	public:
		in_port(uint16_t address) : ADDRESS(address) {
		}

		virtual ~in_port() {
		}

		T read() const {
			volatile uint8_t data = 0;
			asm volatile("inb %0, %%al\t\n"
						"movb %%al, (%1)"
						:
						: "d"(ADDRESS), "r"(&data)
						: "%al");
			return T(data);
		}
	};

	template <class T>
	class in_port<T, 2> {
	private:
		const uint16_t ADDRESS;

	public:
		in_port(uint16_t address) : ADDRESS(address) {
		}

		virtual ~in_port() {
		}

		T read() const {
			volatile uint16_t data = 0;
			asm volatile("inw %0, %%ax\t\n"
						"movw %%ax, (%1)"
						:
						: "d"(ADDRESS), "r"(&data)
						: "%ax");
			return T(data);
		}
	};

	template <class T>
	class in_port<T, 4> {
	private:
		const uint16_t ADDRESS;

	public:
		in_port(uint16_t address) : ADDRESS(address) {
		}

		virtual ~in_port() {
		}

		T read() const {
			volatile uint32_t data = 0;
			asm volatile("inl %0, %%eax\t\n"
						"movl %%eax, (%1)"
						:
						: "d"(ADDRESS), "r"(&data)
						: "%eax");
			return T(data);
		}
	};

	template <class T>
	class out_port<T, 1> {
	private:
		const uint16_t ADDRESS;

	public:
		out_port(uint16_t address) : ADDRESS(address) {
		}

		virtual ~out_port() {
		}

		void write(T data) const {
			asm volatile("outb %1, %0" : : "d"(ADDRESS), "a"((uint8_t) data));
		}
	};

	template <class T>
	class out_port<T, 2> {
	private:
		const uint16_t ADDRESS;

	public:
		out_port(uint16_t address) : ADDRESS(address) {
		}

		virtual ~out_port() {
		}

		void write(T data) const {
			asm volatile("outw %1, %0" : : "d"(ADDRESS), "a"((uint16_t) data));
		}
	};

	template <class T>
	class out_port<T, 4> {
	private:
		const uint16_t ADDRESS;

	public:
		out_port(uint16_t address) : ADDRESS(address) {
		}

		virtual ~out_port() {
		}

		void write(T data) const {
			asm volatile("outl %1, %0" : : "d"(ADDRESS), "a"((uint32_t) data));
		}
	};

	template <class T, size_t BYTES = sizeof(T)>
	class io_port : public in_port<T, BYTES>, public out_port<T, BYTES> {
	public:
		io_port(uint16_t address) : in_port<T, BYTES>(address), out_port<T, BYTES>(address) {
		}

		~io_port() {
		}
	};

} // namespace hal

#endif // DANAOS_MACHINE_IO_PORT_H_