/**
 * machine/devices/pic.cc
**/

#include "pic.h"
#include "main/kernel.h"
// TODO: remove this ASAP!

namespace hal {

    irq_controller::irq_controller() noexcept
        : m_cmd_master(MASTER_PORT),
        m_cmd_slave(SLAVE_PORT),
        m_data_master(MASTER_PORT + 1),
        m_data_slave(SLAVE_PORT + 1),
        m_enabled(false) {
        for (size_t i = 0; i < sizeof(m_irq_gates) / sizeof(isr *); i++) {
            m_irq_gates[i] = nullptr;
        }

        // TODO: is auto EOI a good thing? So far it's off...
        const init_ctrl_word_1 ICW1_MASTER(true, false, false, false, true, 0);
        const init_ctrl_word_1 ICW1_SLAVE(true, false, false, false, true, 0);
        const init_ctrl_word_2 ICW2_MASTER(MASTER_IDT_OFFSET);
        const init_ctrl_word_2 ICW2_SLAVE(MASTER_IDT_OFFSET + 8);
        const init_ctrl_word_3 ICW3_MASTER(static_cast<uint8_t>(irq::SLAVE_IRQ), false);
        const init_ctrl_word_3 ICW3_SLAVE(static_cast<uint8_t>(irq::SLAVE_IRQ), true);
        const init_ctrl_word_4 ICW4_MASTER(true, false, false, false, false);
        const init_ctrl_word_4 ICW4_SLAVE(true, false, false, false, false);

        // Iniitalize both PICs!
        m_cmd_master.write(ICW1_MASTER.get_raw());
        m_cmd_slave.write(ICW1_SLAVE.get_raw());

        // Send the ICW2 (the IDT offsets) to the respective data registers
        m_data_master.write(ICW2_MASTER.get_raw());
        m_data_slave.write(ICW2_SLAVE.get_raw());

        // Set the slave's IRQ in both PICs
        m_data_master.write(ICW3_MASTER.get_raw());
        m_data_slave.write(ICW3_SLAVE.get_raw());

        // If specified, write ICW4 too (e.g. for setting x86)
        if (ICW1_MASTER.get_expects_cw4()) {
            m_data_master.write(ICW4_MASTER.get_raw());
        }
        if (ICW1_SLAVE.get_expects_cw4()) {
            m_data_slave.write(ICW4_SLAVE.get_raw());
        }

        // Clear the data registers to disallow any IRQ
        m_data_master.write(0xFF);
        m_data_slave.write(0xFF);

        // Allow slave interrupts to go through to master
        this->enable(irq::SLAVE_IRQ, *this);
    }

    irq_controller::~irq_controller() noexcept {
        // TODO: disable PIC itself somehow?
    }

    irq_controller &irq_controller::instance() noexcept {
        static irq_controller instance;
        return instance;
    }

    void irq_controller::enable(irq irq, isr &gate) noexcept {
        // TODO: use OCW1 struct
        // TODO: check gate!

        // Set the interrupt gate
        m_irq_gates[static_cast<uint8_t>(irq)] = &gate;

        // Distinguish between master and slave
        if (static_cast<uint8_t>(irq) < 8) {
            // Disable the bit in the mask to allow IRQ
            uint8_t mask = 0xFF & ~(1 << static_cast<uint8_t>(irq));
            mask &= m_data_master.read();
            m_data_master.write(mask);
        } else {
            // Disable the bit in the mask to allow IRQ
            uint8_t mask = 0xFF & ~(1 << (static_cast<uint8_t>(irq) - 8));
            mask &= m_data_slave.read();
            m_data_slave.write(mask);
        }
    }

    void irq_controller::disable(irq irq) noexcept {
        // TODO: use OCW1 struct
        if (irq == irq::SLAVE_IRQ) {
            // We do not allow the slave IRQ to be masked!
            return;
        }

        // Remove the interrupt gate
        m_irq_gates[static_cast<uint8_t>(irq)] = nullptr;

        // Distinguish between master and slave
        if (static_cast<uint8_t>(irq) < 8) {
            // Enable the bit in the mask to disallow IRQ
            uint8_t mask = 1 << static_cast<uint8_t>(irq);
            mask |= m_data_master.read();
            m_data_master.write(mask);
        } else {
            // Disable the bit in the mask to allow IRQ
            uint8_t mask = 1 << (static_cast<uint8_t>(irq) - 8);
            mask |= m_data_slave.read();
            m_data_slave.write(mask);
        }
    }

    bool irq_controller::is_enabled(irq irq) noexcept {
        // TODO: use OCW1 struct
        // Distinguish between master and slave
        if (static_cast<uint8_t>(irq) < 8) {
            // Check if bit is disabled
            uint8_t mask = m_data_master.read();
            return !(mask & (1 << static_cast<uint8_t>(irq)));
        } else {
            // Check if bit is disabled
            uint8_t mask = m_data_slave.read();
            return !(mask & (1 << (static_cast<uint8_t>(irq) - 8)));
        }
    }

    void irq_controller::acknowledge(irq irq) noexcept {
        // TODO: selection necessary/good?
        uint8_t irqNum = static_cast<uint8_t>(irq);
        op_cmd_word_2 ocw2(irqNum, true, true, false);
        if (irqNum >= 8) {
            ocw2.set_irq(irqNum - 8);
            m_cmd_slave.write(ocw2.get_raw());
            // If the slave got the IRQ, the master must have also
            // gotten the interrupt for the slave, so EOI it, too
            ocw2.set_irq(static_cast<uint8_t>(irq::SLAVE_IRQ));
            m_cmd_master.write(ocw2.get_raw());
        } else {
            m_cmd_master.write(ocw2.get_raw());
        }
    }

    bool irq_controller::is_serviced(irq irq) noexcept {
        op_cmd_word_3 ocw3(internal_register::ISR, false, special_mask_action::NONE);

        uint8_t index = static_cast<uint8_t>(irq);
        if (index >= 8) {
            index -= 8;
            m_cmd_slave.write(ocw3.get_raw());
            // Check if the IRQ is set in the ISR
            return m_data_slave.read() & (1 << index);
        } else {
            m_cmd_master.write(ocw3.get_raw());
            // Check if the IRQ is set in the ISR
            return m_data_master.read() & (1 << index);
        }
    }

    void irq_controller::enable() noexcept {
        // Set the gates for all 8 IRQs for both PICs to this
        for (size_t i = 0; i < 8; i++) {
            hal::isr_dispatcher::instance().register_isr(MASTER_IDT_OFFSET + i, *this);
            hal::isr_dispatcher::instance().register_isr(SLAVE_IDT_OFFSET + i, *this);
        }
        m_enabled = true;
    }

    void irq_controller::disable() noexcept {
        // Remove the gates for all 8 IRQs for both PICs
        for (size_t i = 0; i < 8; i++) {
            hal::isr_dispatcher::instance().deregister_isr(MASTER_IDT_OFFSET + i);
            hal::isr_dispatcher::instance().deregister_isr(SLAVE_IDT_OFFSET + i);
        }
        m_enabled = false;
    }
    
    bool irq_controller::is_active() const noexcept {
        return m_enabled;
    }

    void irq_controller::trigger(isr_frame &frame) noexcept {
        size_t irqIndex = frame.int_num - MASTER_IDT_OFFSET;
        if (irqIndex >= 8) {
            // Master/slave do not have to be consecutive in IDT
            irqIndex = frame.int_num - SLAVE_IDT_OFFSET + 8;
        }

        // Check for spurious interrupts
        if (irqIndex == 7 || irqIndex == 15) {
            if (!this->is_serviced(static_cast<irq>(frame.int_num))) {
                // Not set in the ISR -> spurious!
                return;
            }
        }

        // Acknowledge the IRQ
        this->acknowledge(static_cast<irq>(irqIndex));

        bool triggered = false;

        if (this->m_irq_gates[irqIndex] != nullptr) {
            triggered = true;
            this->m_irq_gates[irqIndex]->trigger(frame);
        }

        if (!triggered) {
            // TODO: proper panic ^.^
            kernel::panic("IRQ without gate triggered: {} / {}!", irqIndex, frame.int_num);
        }
    }

} // namespace hal