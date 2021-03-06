#include "system.h"

#define ST_READY 1
#define ST_BUSY 3
#define ST_TRANSMITTED 5
#define ST_RECEIVED 5

#define CMD_ACK 1
#define CMD_TRANSMIT 2
#define CMD_RECEIVE 2

#define CHAR_OFFSET 8
#define TERM_STATUS_MASK 0xFF

static termreg_t *term0_reg = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, 0);

static unsigned int tx_status(termreg_t *tp) {
    return ((tp->transm_status) & TERM_STATUS_MASK);
}

static unsigned int rx_status(termreg_t *tp) {
    return ((tp->recv_status) & TERM_STATUS_MASK);
}

static int term_putchar(char c) {
    unsigned int stat;

    stat = tx_status(term0_reg);
    if (stat != ST_READY && stat != ST_TRANSMITTED)
        return -1;

    term0_reg->transm_command = ((c << CHAR_OFFSET) | CMD_TRANSMIT);

    while ((stat = tx_status(term0_reg)) == ST_BUSY)
        ;

    term0_reg->transm_command = CMD_ACK;

    if (stat != ST_TRANSMITTED)
        return -1;
    else
        return 0;
}

void term_puts(const char *str) {
    while (*str)
        if (term_putchar(*str++))
            return;
}

char term_readc() {
    unsigned int stat;
    char c;

    stat = rx_status(term0_reg);
    if (stat != ST_READY && stat != ST_RECEIVED)
        return -1;

    term0_reg->recv_command = CMD_RECEIVE;

    while ((stat = rx_status(term0_reg)) == ST_BUSY)
        ;
    
    c = term0_reg->recv_status >> CHAR_OFFSET;

    term0_reg->recv_command = CMD_ACK;

    return c;
}

    char c;
int term_gets(char *str, int max) {
    int i = 0;

    do {
        c = term_readc();
        str[i++] = c;
    } while( i< max && c != 0 && c != '\n');

    return i;
}