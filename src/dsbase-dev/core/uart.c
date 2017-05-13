/*
 * uart.c
 *
 *  Created on: Apr 29, 2017
 *      Author: jeffc
 */

#include <msp430.h>
#include <stdio.h>
#include "utils.h"
#include "uart.h"

FILE_STATIC uint8_t txBuff[MAX_BUFFER_SIZE];
FILE_STATIC uint8_t rxBuff[MAX_BUFFER_SIZE];
FILE_STATIC volatile uint8_t currentTxIndex = INDEX_OP_COMPLETE;
FILE_STATIC uint8_t currentTxNumBytes;
FILE_STATIC volatile uint8_t currentRxIndex = INDEX_OP_COMPLETE;
FILE_STATIC uint8_t currentRxNumBytes;

FILE_STATIC bus_status_UART uart_status;

// TODO:  Add macro magic to select which UART peripheral to use
// TODO:  Add configuration parameters for speed
void uartInit()
{
    // Only initialize once
    if (uart_status.initialized != 0)
        return;

    uart_status.initialized = 1;
    BACKCHANNEL_UART_SEL0 &= ~BACKCHANNEL_UART_BITS;
    BACKCHANNEL_UART_SEL1 |= BACKCHANNEL_UART_BITS;

    // TODO:  Add logic to rejigger the dividers based on current clock
    // setting ... these currently ASSUME A 8MHz CLOCK!
    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                    // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 21-4: UCBRSx = 0x04
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA0BRW = 52;                           // 8000000/16/9600
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
    UCA0IE |= UCRXIE | UCTXIE;              // Enable USCI_A0 RX interrupt
}

void uartTransmit(uint8_t * srcBuff, uint8_t szBuff)
{
    // Check for overlong transmission or transmission in progress
    if (currentTxIndex != INDEX_OP_COMPLETE)
    {
        uart_status.txOverrunCount++;
        return;
    }
    if (szBuff > MAX_BUFFER_SIZE)
    {
        uart_status.txBufferOverflowCount++;
        return;
    }

    // Transmit is good, cache the data to be sent
    memcpy(txBuff, srcBuff, szBuff);
    currentTxIndex = 0;
    currentTxNumBytes = szBuff;

    // Start write process
    UCA0TXBUF = txBuff[currentTxIndex++];

    __bis_SR_register(GIE);     // Interrupts enabled

}

#pragma vector=EUSCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:   // Complete character recv'd
            while(!(UCA0IFG&UCTXIFG));
            UCA0TXBUF = UCA0RXBUF;
            __no_operation();
            break;
        case USCI_UART_UCTXIFG:     // Set when tx buff is empty
            if (currentTxIndex < currentTxNumBytes)
            {
                UCA0TXBUF = txBuff[currentTxIndex++];
            }
            else
            {
                currentTxIndex = INDEX_OP_COMPLETE;
            }
            break;
        case USCI_UART_UCSTTIFG: break;  // Set after start received
        case USCI_UART_UCTXCPTIFG: break;
        default: break;
    }
}
