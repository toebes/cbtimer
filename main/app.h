/**
 * @file app.h
 * @author John Toebes (john@toebes.com)
 * @brief
 * @version 0.1
 * @date 2025-05-13
 *
 * @copyright
 * Copyright (c) 2025 John A. Toebes
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _APP_H
#define _APP_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/semphr.h>
#include <led_strip.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_random.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_timer.h>
#include <driver/uart.h>

#ifdef __cplusplus // Provide C++ Compatibility

extern "C"
{
#endif
  /**
   * @brief Application states
   *
   */
  typedef enum
  {
    APP_STATE_CODEBUSTERS,     // We are showing the codebusters logo
    APP_STATE_WAIT_START,      // Displaying the number of minutes left, waiting for the start button
    APP_STATE_TIMED_QUESTION,  // Initial 10 minute interval for the timed question
    APP_STATE_WAIT_25_MINUTES, // Timed question complete, waiting for half way point
    APP_STATE_WAIT_10_MINUTES, // Waiting for 10 minutes remaining
    APP_STATE_WAIT_2_MINUTES,  // Waiting for 2 minutes remaining
    APP_STATE_WAIT_10_SECONDS, // Waiting for the last 10 seconds
    APP_STATE_FINAL_10SECONDS, // Final 10 seconds (showing second timer) waiting for end
    APP_STATE_DONE,            // Test complete
    APP_STATE_CONFIG,          // COnfiguration mode (currently does nothing)
  } APP_STATES;

  /**
   * @brief Configuration for the LEDs on the display
   * Note that each segment consists of several LEDS in the strip and the period is
   * a single LED in the strip.
   */
#define DISPLAY_DIGITS 2
#define DIGIT_SEGMENTS 8
#define BASE_LEDS_PER_SEGMENT 7

// Numbers of the LED in the strip
#define LED_STRIP_TOTAL_LEDS (DISPLAY_DIGITS * (((DIGIT_SEGMENTS - 1) * BASE_LEDS_PER_SEGMENT) + 1))
/**
 * @brief Masks for the segments to display
 *
 */
#define SEG_A (0x01 << 0)
#define SEG_B (0x01 << 1)
#define SEG_C (0x01 << 2)
#define SEG_D (0x01 << 3)
#define SEG_E (0x01 << 4)
#define SEG_F (0x01 << 5)
#define SEG_G (0x01 << 6)
#define SEG_DOT (0x01 << 7)
#define SEG_ALL (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DOT)

#define TICKS_PER_SECOND 24
#define TIMER_INTERVAL (pdMS_TO_TICKS(1000 / TICKS_PER_SECOND))

// GPIO assignments
#define LED_STRIP_PORT 9
#define PUSH_BUTTON_PORT GPIO_NUM_15

#define UART_NUM UART_NUM_1
#define TXD_PIN GPIO_NUM_16
#define RXD_PIN GPIO_NUM_17
  /**
   * @brief
   *
   */
  typedef uint32_t rgb_t;

#define rgb(r, g, b) ((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))
#define RGB_GET_R(color) (((color) >> 16) & 0xff)
#define RGB_GET_G(color) (((color) >> 8) & 0xff)
#define RGB_GET_B(color) ((color) & 0xff)
/**
 * @brief LED Colors to display
 */
#define RGB_ORANGE rgb(255, 165, 0)
#define RGB_RED rgb(255, 0, 0)
#define RGB_GREEN rgb(0, 255, 0)
#define RGB_WHITE rgb(255, 255, 255)
#define RGB_YELLOW rgb(255, 255, 0)
#define RGB_BLACK rgb(0, 0, 0)
#define RGB_PURPLE rgb(255, 0, 255)
#define RGB_BLUE rgb(0, 0, 255)
/**
 * @brief Button press intervals
 */
#define TICKS_PRESSED 2
#define TICKS_DOUBLETAP 10
#define TICKS_REQUEST_RESET (TICKS_PER_SECOND * 2)
#define TICKS_REQUEST_CONFIG (TICKS_PER_SECOND * 5)

#define DFPLAYER_CMD_LENGTH 10
#define DFPLAYER_INIT_DELAY_MS 2000 // Initial power-on wait
#define DFPLAYER_RETRY_DELAY_MS 100 // Retry delay for first command
#define DFPLAYER_RETRY_COUNT 2      // How many times to retry first play command
/**
 * @brief Player tracks
 */
#define TRACK_WELCOME_TO_CODEBUSTERS 2
#define TRACK_NO_MORE_TIMED_BONUS 1
#define TRACK_25_MINUTES_REMAIN 3
#define TRACK_10_MINUTES_REMAIN 4
#define TRACK_2_MINUTES_REMAIN 5
#define TRACK_TIMES_UP 6
/**
 * @brief Time intervals
 */
#if 1
#define END_TIMED_SECONDS (10 * 60)
#define ANNOUNCE_25_MINUTES (25 * 60)
#define ANNOUNCE_10_MINUTES (40 * 60)
#define ANNOUNCE_2_MINUTES (48 * 60)
#define FINAL_SECONDS ((50 * 60) - 10)
#define EVENT_LENGTH (50 * 60)
#define SCALE_SPEED (1)
#else
#define END_TIMED_SECONDS (10)
#define ANNOUNCE_25_MINUTES (15)
#define ANNOUNCE_10_MINUTES (30)
#define ANNOUNCE_2_MINUTES (38)
#define FINAL_SECONDS (40)
#define EVENT_LENGTH (50)
#define SCALE_SPEED (60)
#endif

  typedef struct
  {
    int nPressedCount;                 // Ticks that the button is pressed
    int nReleasedCount;                // Ticks that the button is released
    APP_STATES stateApp;               // Application state
    bool bStartState;                  // Flag indicating that the state was just started
    int64_t tStartTime;                // Time in microseconds that we started
    int64_t tNow;                      // Current time in microseconds
    double dLastSeconds;               // Last time we updated display
    double dElapsedSeconds;            // Total elapsed seconds since start
    uint32_t amDigits[DISPLAY_DIGITS]; // Digits to display
    SemaphoreHandle_t hTimerSemaphore; // Semaphore to run a tick
    led_strip_handle_t ahLEDStrip;     // IO Handle for the LED Strip
  } APP_DATA;

  extern APP_DATA appData;

  extern uint32_t Get_Segment_Mask(int nVal);

  extern led_strip_handle_t configure_led(int gpio);
  extern rgb_t getRGB(void);
  extern void Process_Tick(TimerHandle_t xTimer);
  extern void dfplayer_send_command(uint8_t command, uint16_t param);
  extern void dfplayer_play_track(uint16_t track_num);
  extern void dfplayer_set_volume(uint8_t volume);
  extern void dfplayer_safe_init(uint8_t initial_volume, uint16_t test_track);
  extern void init_uart(void);
  extern void Timer_Display(void);
  extern void HW_Initialize(void);
  extern void APP_Initialize(void);
  extern void ScrollCodebusters(void);
  extern void showSecondsCountdownTime(void);
  extern void showCountdownTime(void);
  extern void Switch_To_State(APP_STATES newState);
  extern bool HandleTimedState(double timeLimit, int track, APP_STATES nextState);
  extern void APP_Main(void);
#endif /* _APP_H */

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

/*******************************************************************************
 End of File
 */