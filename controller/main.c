//**********************************************************************************
//  Copyright 2024 Paul Chote, All Rights Reserved
//**********************************************************************************

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdint.h>
#include "gpio.h"
#include "usb.h"

// Amount of time to power the actuators when opening/closing
#define ACTIVE_SECONDS 30

#define STATE_STOPPED 0
#define STATE_OPEN 1
#define STATE_CLOSED 2

char *labels[] = {
    "STOPPED\r\n",
    "OPEN\r\n",
    "CLOSED\r\n",
    "OPENING\r\n",
    "CLOSING\r\n"
};

uint8_t command_length = 0;
char command_buffer[16];

volatile uint8_t requested_state = STATE_STOPPED;
volatile uint8_t current_state = STATE_STOPPED;
volatile uint8_t move_counter = 0;

gpin_t usb_conn_led = { &PORTC, &PINC, &DDRC, PC7 };
gpin_t usb_rx_led = { &PORTB, &PINB, &DDRB, PB0 };
gpin_t usb_tx_led = { &PORTD, &PIND, &DDRD, PD5 };

gpin_t open_relay = { &PORTB, &PINB, &DDRB, PB6 };
gpin_t close_relay = { &PORTB, &PINB, &DDRB, PB7 };

static void print_string(char *message)
{
    usb_write_data(message, strlen(message));
}

static void request_state(uint8_t state)
{
    requested_state = state;
    cli();
    // TODO: Implement a wear levelling strategy
    eeprom_update_byte((uint8_t*)0, state);
    sei();
}

static void loop(void)
{
    while (usb_can_read())
    {
        int16_t value = usb_read();
        if (value < 0)
            break;

        if (command_length > 0 && (value == '\r' || value == '\n'))
        {
            char *cb = command_buffer;
            if (command_length == 1 && cb[0] == '?')
                print_string(labels[current_state + (move_counter > 0 ? 2 : 0)]);
            else if (command_length == 1 && cb[0] == 'C')
            {
                request_state(STATE_CLOSED);
                print_string("$\r\n");
            }
            else if (command_length == 1 && cb[0] == 'O')
            {
                request_state(STATE_OPEN);
                print_string("$\r\n");
            }
            else if (command_length == 1 && cb[0] == 'S')
            {
                request_state(STATE_STOPPED);
                print_string("$\r\n");
            }
            else
                print_string("?\r\n");

            command_length = 0;
        }
        else
        {
            command_buffer[command_length] = (uint8_t)value;
            if (command_length < sizeof(command_buffer))
                command_length++;
        }
    }
}

int main(void)
{
    // Make sure the relays are disabled before doing anything else
    PORTB = 0;
    gpio_output_set_low(&open_relay);
    gpio_configure_output(&open_relay);
    gpio_output_set_low(&close_relay);
    gpio_configure_output(&close_relay);

    // Configure timer1 to interrupt every second
    OCR1A = 15624;
    TCCR1B = _BV(CS12) | _BV(CS10) | _BV(WGM12);
    TIMSK1 |= _BV(OCIE1A);

    requested_state = current_state = eeprom_read_byte((uint8_t*)0);

    usb_initialize(&usb_conn_led, &usb_rx_led, &usb_tx_led);

    sei();
    for (;;)
        loop();
}

ISR(TIMER1_COMPA_vect)
{
    if (move_counter > 0 && --move_counter == 0)
    {
        gpio_output_set_low(&open_relay);
        gpio_output_set_low(&close_relay);
    }
    
    if (requested_state == current_state)
        return;

    if (current_state != STATE_STOPPED)
    {
        // Make sure both relays are disabled before activating desired relay
        gpio_output_set_low(&open_relay);
        gpio_output_set_low(&close_relay);
        current_state = STATE_STOPPED;
        move_counter = 0;
    }
    else if (requested_state == STATE_OPEN)
    {
        gpio_output_set_high(&open_relay);
        current_state = STATE_OPEN;
        move_counter = ACTIVE_SECONDS;
    }
    else if (requested_state == STATE_CLOSED)
    {
        gpio_output_set_high(&close_relay);
        current_state = STATE_CLOSED;
        move_counter = ACTIVE_SECONDS;
    }
}