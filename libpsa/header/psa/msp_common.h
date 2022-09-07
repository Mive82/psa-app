#ifndef MSP_COMMON_H
#define MSP_COMMON_H

#include "psa_packet_defs.h"
#include <stdint.h>

#define MSP_START_MAGIC 0x69

enum psa_msp_state
{
    MSP_IDLE,      // Waiting for message start
    MSP_GOT_MAGIC, // Waiting for direction
    MSP_GOT_DIR,   // Waiting for ident
    MSP_GOT_IDENT, // Waiting for size
    MSP_GOT_SIZE,  // Waiting for data
    MSP_GOT_DATA,  // Waiting for crc
    MSP_COMPLETE,  // Packet complete
};

struct psa_incoming_msp_packet
{
    uint8_t state;
    uint8_t data_offset;
    uint8_t packet_complete;
    struct psa_header header;
    uint8_t data[254];
    uint8_t crc;
};

// void msp_process_received_data(struct psa_incoming_msp_packet *msp_message, uint8_t byte_read);

#endif