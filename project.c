#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TXD_PIN GPIO_NUM_19
#define RXD_PIN GPIO_NUM_18
#define UART_NUM UART_NUM_2
#define BUF_SIZE 512

// Global traffic data
volatile int lane1_count = 0;
volatile int lane2_count = 0;

// Function to send an AT command and get the response
size_t command(const char* cmd, char* response) {
    char tmp[BUF_SIZE];
    snprintf(tmp, sizeof(tmp), "%s\r\n", cmd);
    uart_write_bytes(UART_NUM, tmp, strlen(tmp));
    size_t len = uart_read_bytes(UART_NUM, response, (BUF_SIZE - 1), 500 / portTICK_PERIOD_MS);
    if (len > 0) response[len] = 0;
    return len;
}

// Emulated red/green signal display
void display_signals(char green_lane) {
    if (green_lane == '1') {
        printf("\n🔴 Lane 2 RED\n🟢 Lane 1 GREEN\n");
    } else {
        printf("\n🔴 Lane 1 RED\n🟢 Lane 2 GREEN\n");
    }
}

// Task to receive UART LoRa data continuously
void uart_receive_task(void *arg) {
    char data[BUF_SIZE];
    while (1) {
        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE - 1, 1000 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = 0;
            char* rcv_start = strstr(data, "+RCV=");
            if (rcv_start) {
                char* payload = strchr(rcv_start, ',');
                if (payload) payload = strchr(payload + 1, ',');
                if (payload) {
                    payload++;
                    int l1 = 0, l2 = 0;
                    if (sscanf(payload, "LANE1:%d,LANE2:%d", &l1, &l2) == 2) {
                        lane1_count = l1;
                        lane2_count = l2;
                        printf("🛰️ Updated Counts -> 🚗 Lane A: %d | 🚙 Lane B: %d\n", lane1_count, lane2_count);
                    }
                }
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// Main signal logic
void handle_signal_logic() {
    char active_lane;
    int green_time;

    if (lane1_count > lane2_count) {
        active_lane = '1';
        green_time = (lane1_count <= 5) ? 20 : 30;
    } else {
        active_lane = '2';
        green_time = (lane2_count <= 5) ? 20 : 30;
    }

    display_signals(active_lane);
    printf("⏱️ Lane %c GREEN for %d seconds\n", active_lane, green_time);

    for (int sec = green_time; sec > 0; sec--) {
        printf("Lane %c: %d sec left | 🚗 Lane A: %d | 🚙 Lane B: %d\n",
               active_lane, sec, lane1_count, lane2_count);

        if (sec == 5) {
            char next_lane = (active_lane == '1') ? '2' : '1';
            int other_lane_count = (next_lane == '1') ? lane1_count : lane2_count;
            int next_time = (other_lane_count <= 5) ? 20 : 30;

            printf("⏭️ Next Lane %c will get GREEN for %d seconds after this\n",
                   next_lane, next_time);

            vTaskDelay(5000 / portTICK_PERIOD_MS); // wait final 5 sec

            display_signals(next_lane);
            printf("🚦 Switched to Lane %c GREEN for %d seconds\n", next_lane, next_time);

            for (int s = next_time; s > 0; s--) {
                printf("Lane %c: %d sec left | 🚗 Lane A: %d | 🚙 Lane B: %d\n",
                       next_lane, s, lane1_count, lane2_count);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }

            return;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    char response[BUF_SIZE];
    if (command("AT+RESET", response))      printf("Reset: %s\n", response);
    if (command("AT+MODE=0", response))     printf("Mode: %s\n", response);
    if (command("AT+ADDRESS=10", response)) printf("Address: %s\n", response);
    if (command("AT+NETWORKID=6", response))printf("Network ID: %s\n", response);
    if (command("AT+BAND=433000000", response)) printf("Band: %s\n", response);
    if (command("AT+PARAMETER=12,7,1,4", response)) printf("Params: %s\n", response);

    uart_flush_input(UART_NUM);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Create UART listener task
    xTaskCreate(uart_receive_task, "uart_receive_task", 4096, NULL, 10, NULL);

    // Main loop: keep cycling through logic every cycle
    while (1) {
        handle_signal_logic();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
