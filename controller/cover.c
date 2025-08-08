/*
 * This file is part of the Robotic Observatory Control Kit (rockit)
 *
 * rockit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rockit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rockit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <pico/stdlib.h>
#include <pico/time.h>

// Amount of time to power the actuators when opening/closing
#define ACTIVE_SECONDS 27

// Open west / close east first to avoid overlap clashes
#define SINGLE_SECONDS 5

// The four relays are connected to GPIO 18-22
// We don't use any other GPIOs so use gpio_put_all with named masks for each state
#define RELAYS_INIT (0b1111 << 18)
#define RELAYS_OPEN_WEST (0b1001 << 18)
#define RELAYS_OPEN_BOTH (0b1101 << 18)
#define RELAYS_CLOSE_EAST (0b0110 << 18)
#define RELAYS_CLOSE_BOTH (0b1110 << 18)
#define RELAYS_IDLE 0

#define STATE_STOPPED 0
#define STATE_OPEN 1
#define STATE_CLOSED 2

char *labels[] = {
    "STOPPED\r",
    "OPEN\r",
    "CLOSED\r",
    "OPENING\r",
    "CLOSING\r"
};

uint8_t command_length = 0;
char command_buffer[16];

volatile uint8_t requested_state = STATE_STOPPED;
volatile uint8_t current_state = STATE_STOPPED;
volatile uint8_t move_counter = 0;

bool timer_cb(struct repeating_timer *t)
{
    if (move_counter == SINGLE_SECONDS)
        gpio_put_all(requested_state == STATE_OPEN ? RELAYS_OPEN_BOTH : RELAYS_CLOSE_BOTH);

    if (move_counter == ACTIVE_SECONDS)
    {
        gpio_put_all(RELAYS_IDLE);
        move_counter = 0;
    }

    if (move_counter > 0)
        move_counter++;

    if (requested_state == current_state)
        return true;
    
    if (requested_state != STATE_STOPPED)
    {
        gpio_put_all(requested_state == STATE_OPEN ? RELAYS_OPEN_WEST : RELAYS_CLOSE_EAST);
        move_counter = 1;
    }
    else
    {
        gpio_put_all(RELAYS_IDLE);
        move_counter = 0;
    }
    
    current_state = requested_state;
}

int main()
{
    stdio_init_all();
    gpio_init_mask(RELAYS_INIT);
    gpio_set_dir_masked(RELAYS_INIT, RELAYS_INIT);
    gpio_put_all(RELAYS_IDLE);

    struct repeating_timer timer;
    add_repeating_timer_ms(-1000, timer_cb, NULL, &timer);

    while (true)
    {
        char value = stdio_getchar();
        if (command_length > 0 && (value == '\r' || value == '\n'))
        {
            char *cb = command_buffer;
            if (command_length == 1 && cb[0] == '?')
                stdio_puts(labels[current_state + (move_counter > 0 ? 2 : 0)]);
            else if (command_length == 1 && cb[0] == 'C')
            {
                requested_state = STATE_CLOSED;
                stdio_puts("$\r");
            }
            else if (command_length == 1 && cb[0] == 'O')
            {
                requested_state = STATE_OPEN;
                stdio_puts("$\r");
            }
            else if (command_length == 1 && cb[0] == 'S')
            {
                requested_state = STATE_STOPPED;
                stdio_puts("$\r");
            }
            else
                stdio_puts("?\r");

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
