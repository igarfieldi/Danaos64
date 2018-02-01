#ifndef DANAOS_KERNEL_ARCH_PC_DEVICES_PIC_H_
#define DANAOS_KERNEL_ARCH_PC_DEVICES_PIC_H_

#include "hal/interrupts/interrupts.h"
#include "io/port.h"
#include "hal/util/bitmap.h"

namespace hal {

    // TODO: const-correctness?
    class irq_controller : public isr {
    public:
        enum class irq : uint8_t {
            PIT       = 0,
            PS2_1     = 1,
            SLAVE_IRQ = 2,
            RS232_2   = 3,
            RS232_1   = 4,
            LPT_2     = 5,
            FLOPPY    = 6,
            LPT_1     = 7,
            RTC       = 8,
            ATA_4     = 10,
            ATA_3     = 11,
            PS2_2     = 12,
            FPU       = 13,
            ATA_1     = 14,
            ATA_2     = 15
        };

    private:
        enum class internal_register : uint8_t { ISR = 2, IRR = 3, NONE = 0 };

        enum class special_mask_action : uint8_t { SET = (2 << 5), RESET = (3 << 5), NONE = 0 };

        class init_ctrl_word_1 {
        private:
            uint8_t m_raw;

        public:
            init_ctrl_word_1(bool expect_cw4, bool single_pic, bool reduced_interval,
                    bool level_triggered, bool start_init, uint8_t iv_addr)
                    : m_raw(expect_cw4 | (single_pic << 1) | (reduced_interval << 2)
                            | (level_triggered << 3) | (start_init << 4) | ((iv_addr & 0x7) << 5)) {}

            constexpr bool get_expects_cw4() const              { return util::get_bit(m_raw, 0); }
            constexpr bool is_single_pic() const                { return util::get_bit(m_raw, 1); }
            constexpr bool has_reduced_call_interval() const    { return util::get_bit(m_raw, 2); }
            constexpr bool is_level_triggered() const           { return util::get_bit(m_raw, 3); }
            constexpr bool is_init_start() const                { return util::get_bit(m_raw, 4); }
            constexpr uint8_t get_iv_addr() const               { return m_raw >> 5; }
            
            void set_expects_cw4(bool val)                      { m_raw = util::set_bit(m_raw, 0, val); }
            void set_single_pic(bool val)                       { m_raw = util::set_bit(m_raw, 1, val); }
            void set_reduced_call_interval(bool val)            { m_raw = util::set_bit(m_raw, 2, val); }
            void set_level_triggered(bool val)                  { m_raw = util::set_bit(m_raw, 3, val); }
            void set_init_start(bool val)                       { m_raw = util::set_bit(m_raw, 4, val); }
            void set_iv_addr(uint8_t addr)                      { m_raw = util::set_bits(m_raw, 5, 3, uint8_t((addr & 0x7) << 5)); }

            constexpr uint8_t get_raw() const                   { return m_raw; }
        } __attribute__((packed));

        class init_ctrl_word_2 {
        private:
            uint8_t m_idt_offset;

        public:
            init_ctrl_word_2(uint8_t offset) : m_idt_offset(offset) {}

            constexpr uint8_t get_idt_offset() const            { return m_idt_offset; }
            void set_idt_offset(uint8_t offset)                 { m_idt_offset = offset; }
            
            constexpr uint8_t get_raw() const                   { return m_idt_offset; }
        } __attribute__((packed));

        class init_ctrl_word_3 {
        private:
            uint8_t m_slave_irq;

        public:
            // If the word is to be send to the master, the IRQ number
            // isn't specified directly but instead the bit of it
            init_ctrl_word_3(uint8_t irq, bool is_slave) : m_slave_irq() {
                set_slave_irq(irq, is_slave);
            }

            constexpr uint8_t get_slave_irq() const             { return m_slave_irq; }
            void set_slave_irq(uint8_t irq, bool is_slave) {
                if(is_slave) {
                    m_slave_irq = irq;
                } else {
                    m_slave_irq = 1 << (irq & 0x7);
                }
            }

            constexpr uint8_t get_raw() const                   { return m_slave_irq; }
        } __attribute__((packed));

        class init_ctrl_word_4 {
        private:
            uint8_t m_raw;

        public:
            init_ctrl_word_4(bool x86_mode, bool auto_eoi, bool sel_buffer_master,
                    bool buffered_mode, bool fully_nested)
                    : m_raw(x86_mode | (auto_eoi << 1) | (sel_buffer_master << 2)
                            | (buffered_mode << 3) | (fully_nested << 4)) {}

            constexpr bool is_x86_mode() const                  { return util::get_bit(m_raw, 0); }
            constexpr bool is_auto_eoi() const                  { return util::get_bit(m_raw, 1); }
            constexpr bool is_buffer_master() const             { return util::get_bit(m_raw, 2); }
            constexpr bool is_buffered_mode() const             { return util::get_bit(m_raw, 3); }
            constexpr bool is_fully_nested() const              { return util::get_bit(m_raw, 4); }
            
            void set_x86_mode(bool val)                         { m_raw = util::set_bit(m_raw, 0, val); }
            void set_auto_eoi(bool val)                         { m_raw = util::set_bit(m_raw, 1, val); }
            void set_buffer_master(bool val)                    { m_raw = util::set_bit(m_raw, 2, val); }
            void set_buffered_mode(bool val)                    { m_raw = util::set_bit(m_raw, 3, val); }
            void set_fully_nested(bool val)                     { m_raw = util::set_bit(m_raw, 4, val); }

            constexpr uint8_t get_raw() const                   { return m_raw; }
        } __attribute__((packed));
        
        class op_cmd_word_1 {
        private:
            uint8_t m_raw;

        public:
            op_cmd_word_1(uint8_t raw) : m_raw(raw) {}

            constexpr bool get_irq(uint8_t irq) const           { return util::get_bit(m_raw, irq); }
            void set_irq(uint8_t irq, bool val)                 { m_raw = util::set_bit(m_raw, irq, val); }

            constexpr uint8_t get_raw() const                   { return m_raw; }
        } __attribute__((packed));

        class op_cmd_word_2 {
        private:
            uint8_t m_raw;

        public:
            op_cmd_word_2(uint8_t irq, bool eoi, bool selection, bool rotation)
                    : m_raw((irq & 0x7) | (eoi << 5) | (selection << 6) | (rotation << 7)) {}

            constexpr bool get_irq() const                      { return m_raw & 0x7; }
            constexpr bool is_eoi() const                       { return util::get_bit(m_raw, 5); }
            constexpr bool is_selection() const                 { return util::get_bit(m_raw, 6); }
            constexpr bool is_rotation() const                  { return util::get_bit(m_raw, 7); }
            
            void set_irq(uint8_t irq)                           { m_raw = util::set_bits(m_raw, 0, 3, irq); }
            void set_eoi(bool val)                              { m_raw = util::set_bit(m_raw, 5, val); }
            void set_selection(bool val)                        { m_raw = util::set_bit(m_raw, 6, val); }
            void set_rotation(bool val)                         { m_raw = util::set_bit(m_raw, 7, val); }
            
            constexpr uint8_t get_raw() const                   { return m_raw; }
        } __attribute__((packed));

        class op_cmd_word_3 {
        private:
            uint8_t m_raw;

        public:
            op_cmd_word_3(internal_register reg, bool poll, special_mask_action mask)
                    : m_raw(static_cast<uint8_t>(reg) | (poll << 2) | static_cast<uint8_t>(mask)) {}

            constexpr internal_register get_reg() const         { return static_cast<internal_register>(m_raw & 0x3); }
            constexpr bool is_poll() const                      { return util::get_bit(m_raw, 2); }
            constexpr special_mask_action get_mask() const      { return static_cast<special_mask_action>(m_raw & 0x60); }
            
            void set_reg(internal_register reg)                 { m_raw = util::set_bits(m_raw, 0, 2, static_cast<uint8_t>(reg)); }
            void set_poll(bool val)                             { m_raw = util::set_bit(m_raw, 2, val); }
            void set_mask(special_mask_action mask)             { m_raw = util::set_bits(m_raw, 5, 2, static_cast<uint8_t>(mask)); }
            
            constexpr uint8_t get_raw() const                   { return m_raw; }
        } __attribute__((packed));

        static constexpr uint16_t MASTER_PORT      = 0x20;
        static constexpr uint16_t SLAVE_PORT       = 0xA0;
        static constexpr uint8_t MASTER_IDT_OFFSET = 32;
        static constexpr uint8_t SLAVE_IDT_OFFSET  = MASTER_IDT_OFFSET + 8;

        out_port<uint8_t> m_cmd_master;
        out_port<uint8_t> m_cmd_slave;
        io_port<uint8_t> m_data_master;
        io_port<uint8_t> m_data_slave;

        bool m_enabled;
        isr *m_irq_gates[16];

        irq_controller() noexcept;
        void acknowledge(irq irq) noexcept;
        bool is_serviced(irq irq) noexcept;
        

    public:
        irq_controller(const irq_controller &) = delete;
        irq_controller(irq_controller &&) = delete;
        irq_controller &operator=(const irq_controller &) = delete;
        irq_controller &operator=(irq_controller &&) = delete;
        ~irq_controller() noexcept;

        static irq_controller &instance() noexcept;

        void enable(irq irq, isr &gate) noexcept;
        void disable(irq irq) noexcept;
        bool is_enabled(irq irq) noexcept;

        virtual void enable() noexcept override;
        virtual void disable() noexcept override;
        virtual bool is_active() const noexcept override;
        virtual void trigger(isr_frame &frame) noexcept override;
    };

} // namespace hal

#endif //DANAOS_KERNEL_ARCH_PC_DEVICES_PIC_H_