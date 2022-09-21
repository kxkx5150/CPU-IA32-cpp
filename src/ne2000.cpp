#include "devices.h"
#include "io.h"
#include "net.h"
#include "pc.h"
#include <cstdint>
#include <string.h>

#define NE2K_LOG(x, ...)   LOG("NE2K", x, ##__VA_ARGS__)
#define NE2K_DEBUG(x, ...) LOG("NE2K", x, ##__VA_ARGS__)
#define NE2K_FATAL(x, ...) FATAL("NE2K", x, ##__VA_ARGS__)

#define CMD_PAGESEL 0xC0
#define CMD_RWMODE  0x38

#define CMD_MODE_READ  1
#define CMD_MODE_WRITE 2
#define CMD_MODE_SEND  3

#define CMD_TXP 0x04
#define CMD_STA 0x02
#define CMD_STP 0x01
#define ISR_PRX 0x01
#define ISR_PTX 0x02
#define ISR_RXE 0x04
#define ISR_TXE 0x08
#define ISR_OVW 0x10
#define ISR_CNT 0x20
#define ISR_RDC 0x40
#define ISR_RST 0x80
#define DCR_WTS 0x01
#define DCR_BOS 0x02
#define DCR_LAS 0x04
#define DCR_LS  0x08
#define DCR_AR  0x10

#define DCR_FIFO_THRESH 0x60
#define NE2K_DEVID      5
#define NE2K_MEMSTART
#define NE2K_MEMSZ (32 << 10)


struct ne2000
{
    uint32_t iobase;
    int      irq;
    uint8_t  mem[256 * 128];
    uint8_t  cntr[3];
    uint8_t  isr, dcr, imr, rcr, tcr;
    uint8_t  tsr;
    uint8_t  rsr;
    uint16_t rbcr, rsar;
    int      tpsr;
    int      tcnt;
    int      bnry;
    int      curr;
    uint8_t  par[6];
    uint8_t  multicast[8];
    int      pagestart, pagestop;
    int      cmd;
    int      enabled;
} ne2000;

static const uint8_t ne2000_config_space[16] = {0xec, 0x10, 0x29, 0x80, 0x01, 0x00, 0x00, 0x02,
                                                0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00};


static void ne2000_reset_internal(int software)
{
    if (software) {
        ne2000.isr = ISR_RST;
        return;
    } else {
        ne2000.pagestart = 0x40 << 8;
        ne2000.pagestop  = 0x80 << 8;
        ne2000.bnry      = 0x4C << 8;
        ne2000.cmd       = CMD_STP;
    }
}
static void ne2000_reset(void)
{
    ne2000_reset_internal(0);
}
static void ne2000_trigger_irq(int flag)
{
    ne2000.isr |= flag;
    if (!((ne2000.isr & ne2000.imr) & flag))
        return;
    NE2K_DEBUG("Triggering IRQ! (isr=%02x imr=%02x &=%02x)\n", ne2000.isr, ne2000.imr, ne2000.isr & ne2000.imr);
    pci_set_irq_line(NE2K_DEVID, 0);
    pci_set_irq_line(NE2K_DEVID, 1);
}
static void ne2000_lower_irq(void)
{
    pci_set_irq_line(NE2K_DEVID, 0);
}
static uint32_t ne2000_asic_mem_read(void)
{
    if (!(ne2000.dcr & DCR_WTS)) {
        if (ne2000.rsar >= NE2K_MEMSZ)
            return 0xFF;
        uint8_t data = ne2000.mem[ne2000.rsar++];
        ne2000.rbcr--;
        if (!ne2000.rbcr)
            ne2000_trigger_irq(ISR_RDC);
        return data;
    } else {
        if (ne2000.rsar & 1) {
            NE2K_LOG("Unaligned RSAR -- forcing alignment\n");
            ne2000.rsar &= ~1;
        }
        if (ne2000.rsar >= NE2K_MEMSZ)
            return 0xFFFF;
        uint16_t data = ne2000.mem[ne2000.rsar] | (ne2000.mem[ne2000.rsar + 1] << 8);
        ne2000.rsar += 2;
        ne2000.rbcr -= 2;
        if (!ne2000.rbcr)
            ne2000_trigger_irq(ISR_RDC);
        return data;
    }
}
static void ne2000_asic_mem_write(uint32_t data)
{
    if (!(ne2000.dcr & DCR_WTS)) {
        if (ne2000.rsar >= NE2K_MEMSZ)
            return;
        ne2000.mem[ne2000.rsar++] = data;
        ne2000.rbcr--;
        if (!ne2000.rbcr)
            ne2000_trigger_irq(ISR_RDC);
    } else {
        if (ne2000.rsar & 1) {
            NE2K_LOG("Unaligned RSAR -- forcing alignment\n");
            ne2000.rsar &= ~1;
        }
        if (ne2000.rsar >= NE2K_MEMSZ)
            return;
        ne2000.mem[ne2000.rsar]     = data;
        ne2000.mem[ne2000.rsar + 1] = data >> 8;
        ne2000.rsar += 2;
        ne2000.rbcr -= 2;
        if (!ne2000.rbcr)
            ne2000_trigger_irq(ISR_RDC);
    }
}
static uint32_t ne2000_read0(uint32_t port)
{
    uint8_t retv;
    switch (port) {
        case 3:
            retv = ne2000.bnry;
            NE2K_DEBUG("Boundary read: %02x\n", retv);
            break;
        case 4:
            retv = ne2000.tsr;
            NE2K_DEBUG("Trans. status: %02x\n", retv);
            break;
        case 7:
            retv = ne2000.isr;
            NE2K_DEBUG("ISR read: %02x\n", retv);
            break;
        case 13:
        case 14:
        case 15:
            retv = ne2000.cntr[port - 13];
            NE2K_DEBUG("CNTR%d: read %02x\n", port - 13, retv);
            break;
        default:
            NE2K_FATAL("PAGE0 read %02x\n", port);
    }
    return retv;
}
static uint32_t ne2000_read1(uint32_t port)
{
    uint8_t retv;
    switch (port) {
        case 1 ... 6:
            retv = ne2000.par[port - 1];
            NE2K_DEBUG("PAR%d: read %02x\n", port - 1, retv);
            break;
        case 7:
            retv = ne2000.curr >> 8;
            NE2K_DEBUG("CURR: read %02x\n", retv);
            break;
        case 8 ... 15:
            retv = ne2000.multicast[port & 7];
            NE2K_DEBUG("MULTI%d: read %02x\n", port & 7, retv);
            break;
    }
    return retv;
}
static uint32_t ne2000_read(uint32_t port)
{
    switch (port & 31) {
        case 0:
            NE2K_DEBUG("CMD: read %02x\n", ne2000.cmd);
            return ne2000.cmd;
        case 1 ... 15:
            switch (ne2000.cmd >> 6 & 3) {
                case 0:
                    return ne2000_read0(port & 31);
                case 1:
                    return ne2000_read1(port & 31);
                default:
                    NE2K_FATAL("todo: (offs %02x) implement page %d\n", port & 31, ne2000.cmd >> 6 & 3);
            }
            break;
        case 16:
            return ne2000_asic_mem_read();
        case 31:
            return 0;
        default:
            NE2K_FATAL("TODO: read port=%08x\n", port);
    }
}
static uint32_t ne2000_read_mem16(uint32_t port)
{
    UNUSED(port);
    if (ne2000.dcr & DCR_WTS)
        return ne2000_asic_mem_read();
    else {
        int retval = ne2000_asic_mem_read();
        return retval | (ne2000_asic_mem_read() << 8);
    }
}
static uint32_t ne2000_read_mem32(uint32_t port)
{
    UNUSED(port);
    if (ne2000.dcr & DCR_WTS) {
        int retval = ne2000_asic_mem_read();
        retval |= ne2000_asic_mem_read() << 16;
        return retval;
    } else {
        int retval = ne2000_asic_mem_read();
        retval |= ne2000_asic_mem_read() << 8;
        retval |= ne2000_asic_mem_read() << 16;
        retval |= ne2000_asic_mem_read() << 24;
        return retval;
    }
}
static void ne2000_write0(uint32_t port, uint32_t data)
{
    switch (port) {
        case 1:
            NE2K_DEBUG("PageStart write: %02x\n", data);
            ne2000.pagestart = (data << 8) & 0xFF00;
            break;
        case 2:
            NE2K_DEBUG("PageStop write: %02x\n", data);
            ne2000.pagestop = (data << 8) & 0xFF00;
            break;
        case 3:
            NE2K_DEBUG("Boundary write: %02x\n", data);
            ne2000.bnry = data;
            break;
        case 4:
            NE2K_DEBUG("TPSR: %02x\n", data);
            ne2000.tpsr = (data << 8) & 0xFF00;
            break;
        case 5:
        case 6: {
            int b0      = (port & 1) << 3;
            ne2000.tcnt = (ne2000.tcnt & (0xFF << b0)) | (data << (b0 ^ 8));
            NE2K_DEBUG("TCNT: %04x\n", ne2000.tcnt);
            break;
        }
        case 7:
            ne2000.isr &= ~data;
            if (!data)
                ne2000_lower_irq();
            NE2K_DEBUG("ISR ack: %02x\n", ne2000.isr);
            ne2000_trigger_irq(0);
            break;
        case 8:
            NE2K_DEBUG("RSAR low: %02x\n", data);
            ne2000.rsar = (ne2000.rsar & 0xFF00) | data;
            break;
        case 9:
            NE2K_DEBUG("RSAR high: %02x\n", data);
            ne2000.rsar = (ne2000.rsar & 0x00FF) | (data << 8);
            break;
        case 10:
            NE2K_DEBUG("RBCR low: %02x\n", data);
            ne2000.rbcr = (ne2000.rbcr & 0xFF00) | data;
            break;
        case 11:
            NE2K_DEBUG("RBCR high: %02x\n", data);
            ne2000.rbcr = (ne2000.rbcr & 0x00FF) | (data << 8);
            break;
        case 12:
            NE2K_DEBUG("RCR: %02x\n", data);
            ne2000.rcr = data;
            break;
        case 13:
            NE2K_DEBUG("TCR: %02x\n", data);
            ne2000.tcr = data;
            break;
        case 14:
            NE2K_DEBUG("DCR write: %02x\n", data);
            ne2000.dcr = data;
            break;
        case 15:
            NE2K_DEBUG("IMR write: %02x\n", data);
            ne2000.imr = data;
            break;
        case 16:
            ne2000_asic_mem_write(data);
            break;
        default:
            NE2K_FATAL("todo: page0 implement write %d\n", port & 31);
    }
}
static void ne2000_write1(uint32_t port, uint32_t data)
{
    switch (port) {
        case 1 ... 6:
            ne2000.par[port - 1] = data;
            NE2K_DEBUG("PAR%d: %02x\n", port - 1, data);
            break;
        case 7:
            ne2000.curr = data << 8;
            NE2K_DEBUG("CURR: read %02x\n", data);
            break;
        case 8 ... 15:
            ne2000.multicast[port & 7] = data;
            NE2K_DEBUG("Multicast%d: %02x\n", port & 7, data);
            break;
        default:
            NE2K_FATAL("todo: page1 implement port %d\n", port & 31);
    }
}
static void ne2000_write(uint32_t port, uint32_t data)
{
    switch (port & 31) {
        case 0: {
            NE2K_DEBUG("CMD: write %02x\n", data);
            int stop = data & CMD_STP, start = data & CMD_STA, transmit_packet = data & CMD_TXP,
                rdma_cmd = data >> 3 & 7, psel = data >> 6 & 3;
            ne2000.cmd = data;
            UNUSED(psel);
            UNUSED(start);
            if (!stop) {
                if ((rdma_cmd == CMD_MODE_READ) && !ne2000.rbcr) {
                    ne2000_trigger_irq(ISR_RDC);
                }
                if (transmit_packet) {
                    ne2000.tpsr &= NE2K_MEMSZ - 1;
                    if ((ne2000.tpsr + ne2000.tcnt) >= NE2K_MEMSZ) {
                        NE2K_LOG("TRANSMIT ERROR: read past end of memory\n");
                        ne2000_trigger_irq(ISR_TXE);
                    }
                    net_send(&ne2000.mem[ne2000.tpsr], ne2000.tcnt);
                    ne2000.tsr |= 1;
                    ne2000_trigger_irq(ISR_PTX);
                }
                break;
            }
            break;
        }
        case 1 ... 15:
            switch (ne2000.cmd >> 6 & 3) {
                case 0:
                    ne2000_write0(port & 31, data);
                    break;
                case 1:
                    ne2000_write1(port & 31, data);
                    break;
                default:
                    NE2K_FATAL("todo: (offs %d/data %02x) implement page %d\n", port & 31, data, ne2000.cmd >> 6 & 3);
            }
            break;
        case 31:
            NE2K_LOG("Software reset\n");
            ne2000_reset_internal(1);
            break;
        default:
            NE2K_FATAL("TODO: write port=%08x data=%02x\n", port, data);
    }
}
static void ne2000_write_mem16(uint32_t port, uint32_t data)
{
    UNUSED(port);
    if (ne2000.dcr & DCR_WTS)
        ne2000_asic_mem_write(data);
    else {
        ne2000_asic_mem_write(data);
        ne2000_asic_mem_write(data >> 8);
    }
}
static void ne2000_write_mem32(uint32_t port, uint32_t data)
{
    UNUSED(port);
    if (ne2000.dcr & DCR_WTS) {
        ne2000_asic_mem_write(data);
        ne2000_asic_mem_write(data >> 16);
    } else {
        ne2000_asic_mem_write(data);
        ne2000_asic_mem_write(data >> 8);
        ne2000_asic_mem_write(data >> 16);
        ne2000_asic_mem_write(data >> 24);
    }
}
static void ne2000_pci_remap(uint8_t *dev, unsigned int newbase)
{
    if (newbase != ne2000.iobase) {
        dev[0x10] = newbase | 1;
        dev[0x11] = newbase >> 8;
        if (ne2000.iobase != 0) {
            io_unregister_read(ne2000.iobase, 32);
            io_unregister_write(ne2000.iobase, 32);
        }
        io_register_read(newbase, 32, ne2000_read, NULL, NULL);
        io_register_write(newbase, 32, ne2000_write, NULL, NULL);
        io_register_read(newbase + 16, 1, ne2000_read, ne2000_read_mem16, ne2000_read_mem32);
        io_register_write(newbase + 16, 1, ne2000_write, ne2000_write_mem16, ne2000_write_mem32);
        ne2000.iobase = newbase;
        NE2K_LOG("Remapped controller to 0x%x\n", ne2000.iobase);
    }
}
static int ne2000_pci_write(uint8_t *ptr, uint8_t addr, uint8_t data)
{
    switch (addr) {
        case 4:
            ptr[0x04] = data & 3;
            return 1;
        case 5 ... 7:
            return 0;
        case 0x10:
            ptr[0x10] = data | 1;
            return 1;
        case 0x11: {
            ptr[0x11]            = data;
            unsigned int newbase = (ptr[0x10] | (data << 8));
            if (newbase != 0xFFFE) {
                if (newbase & 1)
                    ne2000_pci_remap(ptr, newbase & ~31);
            }
            return 1;
        }
        case 0x12 ... 0x13:
            return 0;
        case 0x14 ... 0x1F:
            return 0;
        case 0x20 ... 0x2F:
            return 0;
        case 0x30 ... 0x33:
            return 1;
        case 0x3C:
            return 0;
        default:
            NE2K_FATAL("unknown pci value: offs=0x%02x data=%02x\n", addr, data);
    }
}
static void ne2000_pci_init(struct ne2000_settings *conf)
{
    uint8_t *dev = (uint8_t *)pci_create_device(0, NE2K_DEVID, 0, ne2000_pci_write);
    pci_copy_default_configuration(dev, (void *)ne2000_config_space, 16);
    dev[0x3D] = 1;
    ne2000_pci_remap(dev, conf->port_base);
}
static void ne2000_receive(void *data, int len)
{
    if (ne2000.cmd & CMD_STP)
        return;
    int length_plus_header = 4 + len;
    int total_pages        = (length_plus_header + 255) >> 8;
    int start              = ne2000.curr;
    int nextpg             = ne2000.curr + ((255 + length_plus_header) & ~0xFF);
    if (nextpg >= ne2000.pagestop) {
        nextpg += ne2000.pagestart - ne2000.pagestop;
    }
    ne2000.rsr        = 1;
    uint8_t *memstart = ne2000.mem + start, *data8 = (uint8_t *)data;
    if (data8[0] & 1)
        ne2000.rsr |= 0x20;
    memstart[0] = ne2000.rsr;
    memstart[1] = nextpg >> 8;
    memstart[2] = length_plus_header;
    memstart[3] = length_plus_header >> 8;
    if ((ne2000.curr < nextpg) || (ne2000.curr + total_pages == ne2000.pagestop)) {
        memcpy(memstart + 4, data, len);
    } else {
        int len1 = ne2000.pagestop - ne2000.curr;
        memcpy(memstart + 4, data, len1 - 4);
        memstart = ne2000.mem + ne2000.pagestart;
        printf("addr=%p len=%d\n", (uint8_t *)data + (len1 - 4), len - (len1 + 4));
        if ((len - (len1 + 4)) < 0)
            __asm__("int3");
        memcpy(memstart, (uint8_t *)data + (len1 - 4), len - (len1 + 4));
    }
    ne2000.curr = nextpg;
    ne2000_trigger_irq(ISR_PRX);
}
void ne2000_poll(void)
{
    if (!ne2000.enabled)
        return;
}
void ne2000_init(struct ne2000_settings *conf)
{
    if (!conf->enabled)
        return;
    ne2000.enabled = 1;
    if (conf->irq == 0)
        conf->irq = 3;
    int macsum = 0;
    for (int i = 0; i < 6; i++)
        macsum |= conf->mac_address[i];
    if (macsum == 0) {
        conf->mac_address[0] = 0x12;
        conf->mac_address[1] = 0x34;
        conf->mac_address[2] = 0x56;
        conf->mac_address[3] = 0x78;
        conf->mac_address[4] = 0x9A;
        conf->mac_address[5] = 0xBC;
    }
    for (int i = 0; i < 8; i++) {
        int val;
        if (i == 7)
            val = 0x57;
        else
            val = conf->mac_address[i];
        ne2000.mem[i << 1] = ne2000.mem[(i << 1) | 1] = val;
    }
    if (conf->pci) {
        ne2000_pci_init(conf);
    } else {
        ne2000.iobase = conf->port_base & ~31;
        ne2000.irq    = conf->irq & 15;
        io_register_read(0x300, 32, ne2000_read, NULL, NULL);
        io_register_write(0x300, 32, ne2000_write, NULL, NULL);
        io_register_read(0x300 + 16, 1, ne2000_read, ne2000_read_mem16, ne2000_read_mem32);
        io_register_write(0x300 + 16, 1, ne2000_write, ne2000_write_mem16, ne2000_write_mem32);
    }
    io_register_reset(ne2000_reset);
}