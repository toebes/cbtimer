/**
 * @file app.c
 * @author John TOebes (john@toebes.com.com)
 * @brief
 * @version 0.1
 * @date 2025-05-13
 *
 * @copyright Copyright (c) 2025 John A. Toebes, All Rights Reserved
 *
 */
#include "app.h"
static const char *TAG = "app";

APP_DATA appData;
/**
 * @brief Return which Segments correspond to a given letter
 *
 * @param nVal Letter/value to look up
 * @return uint32_t Mask of segments to turn on
 */
uint32_t Get_Segment_Mask(int nVal)
{
    switch (nVal)
    {
    case 0:
    case 'O':
    case 'o':
        return (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F);
    case 1:
        return (SEG_B | SEG_C);
    case 2:
        return (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G);
    case 3:
        return (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G);
    case 4:
        return (SEG_B | SEG_C | SEG_F | SEG_G);
    case 5:
    case 'S':
    case 's':
        return (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G);
    case 6:
        return (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);
    case 7:
    case 'T':
    case 't':
        return (SEG_A | SEG_B | SEG_C);
    case 8:
        return (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);
    case 9:
    case 'g':
    case 'G':
        return (SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G);
    case 10:
    case 'A':
    case 'a':
        return (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G);
    case 11:
    case 'B':
    case 'b':
        return (SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);
    case 12:
    case 'C':
    case 'c':
        return (SEG_A | SEG_D | SEG_E | SEG_F);
    case 13:
    case 'D':
    case 'd':
        return (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G);
    case 14:
    case 'E':
    case 'e':
        return (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G);
    case 15:
    case 'F':
    case 'f':
        return (SEG_A | SEG_E | SEG_F | SEG_G);
    case 'R':
    case 'r':
        return (SEG_E | SEG_G);
    case 'u':
        return (SEG_C | SEG_D | SEG_E);
    case 'U':
        return (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F);
    case '^':
        return SEG_A;
    case ' ':
        return 0;
    case '.':
        return SEG_DOT;
    case '?':
    default:
        return (SEG_A | SEG_B | SEG_E | SEG_G | SEG_DOT);
    }
}
/**
 * @brief Determine the color to display in
 *
 * @return rgb_t RGB value corresponding to the state of the application
 */
rgb_t getRGB(void)
{
    switch (appData.stateApp)
    {
    case APP_STATE_CODEBUSTERS:
        return RGB_ORANGE;

    case APP_STATE_WAIT_START:
    case APP_STATE_DONE:
        return RGB_RED;

    case APP_STATE_TIMED_QUESTION:
        return RGB_GREEN;

    case APP_STATE_WAIT_25_MINUTES:
    case APP_STATE_WAIT_10_MINUTES:
    case APP_STATE_WAIT_2_MINUTES:
    case APP_STATE_WAIT_10_SECONDS:
        return RGB_WHITE;

    case APP_STATE_FINAL_10SECONDS:
        return RGB_YELLOW;

    case APP_STATE_CONFIG:
        return RGB_PURPLE;
    }
    return RGB_BLACK;
}
/**
 * @brief Handles the timer callback for the timer to check button presses.
 *
 * @param xTimer Timer handle for the callback
 */
void Process_Tick(TimerHandle_t xTimer)
{
    if (gpio_get_level(PUSH_BUTTON_PORT) == 0)
    {
        appData.nPressedCount++;
        if (appData.nReleasedCount > 0 &&
            appData.nReleasedCount < TICKS_DOUBLETAP)
        {
            Switch_To_State(APP_STATE_DONE);
        }
        else if (appData.nPressedCount == TICKS_PRESSED)
        {
            if (appData.stateApp == APP_STATE_WAIT_START)
            {
                ESP_LOGI(TAG, "Starting Event");
                Switch_To_State(APP_STATE_TIMED_QUESTION);
            }
            else if (appData.stateApp == APP_STATE_DONE)
            {
                ESP_LOGI(TAG, "Going to Codebusters");
                Switch_To_State(APP_STATE_CODEBUSTERS);
            }
        }
        else if (appData.nPressedCount == TICKS_REQUEST_RESET &&
                 appData.stateApp != APP_STATE_WAIT_START)
        {
            ESP_LOGI(TAG, "Reset to Wait State");
            Switch_To_State(APP_STATE_WAIT_START);
        }
        else if (appData.nPressedCount == TICKS_REQUEST_CONFIG &&
                 appData.stateApp == APP_STATE_CODEBUSTERS)
        {
            ESP_LOGI(TAG, "Going to Config State");
            Switch_To_State(APP_STATE_CONFIG);
        }
        appData.nReleasedCount = 0;
    }
    else
    {
        appData.nPressedCount = 0;
        appData.nReleasedCount++;
    }
    xSemaphoreGiveFromISR(appData.hTimerSemaphore, NULL);
}

/**
 * @brief Compute checksum (2's complement of sum of bytes 1 to 6)
 *
 * @param cmd Command bytes to checksum
 * @return uint16_t Place to put checksum value
 */
static uint16_t dfplayer_checksum(uint8_t *cmd)
{
    uint16_t sum = 0;
    for (int i = 1; i < 7; i++)
    {
        sum += cmd[i];
    }
    return 0xFFFF - sum + 1;
}

/**
 * @brief Send DFPlayer command with checksum
 *
 * @param command Command to send to the player
 * @param param Parameter for the command
 */
void dfplayer_send_command(uint8_t command, uint16_t param)
{
    uint8_t cmd[DFPLAYER_CMD_LENGTH] = {0};

    cmd[0] = 0x7E;                // Start byte
    cmd[1] = 0xFF;                // Version
    cmd[2] = 0x06;                // Length
    cmd[3] = command;             // Command
    cmd[4] = 0x00;                // No feedback
    cmd[5] = (param >> 8) & 0xFF; // High byte of param
    cmd[6] = param & 0xFF;        // Low byte of param

    uint16_t checksum = dfplayer_checksum(cmd);
    cmd[7] = (checksum >> 8) & 0xFF; // Checksum high byte
    cmd[8] = checksum & 0xFF;        // Checksum low byte
    cmd[9] = 0xEF;                   // End byte

    uart_write_bytes(UART_NUM_1, (const char *)cmd, sizeof(cmd));

    ESP_LOG_BUFFER_HEX("DFPlayer CMD", cmd, sizeof(cmd));
}
/**
 * @brief Play a specific track number
 *
 * @param track_num Which track number to play
 */
void dfplayer_play_track(uint16_t track_num)
{
    dfplayer_send_command(0x03, track_num);
}
/**
 * @brief Set Volume
 *
 * @param volume Volume (0-30)
 */
void dfplayer_set_volume(uint8_t volume)
{
    dfplayer_send_command(0x06, volume);
}
void dfplayer_safe_init(uint8_t initial_volume, uint16_t test_track)
{
    ESP_LOGI("DFPlayer", "Initializing DFPlayer (waiting %d ms for power stabilization)...", DFPLAYER_INIT_DELAY_MS);
    vTaskDelay(pdMS_TO_TICKS(DFPLAYER_INIT_DELAY_MS));

    // Set initial volume
    dfplayer_set_volume(initial_volume);
    vTaskDelay(pdMS_TO_TICKS(100)); // Small delay after volume set

    // Try playing the test track with retry logic
    for (int i = 0; i < DFPLAYER_RETRY_COUNT; i++)
    {
        ESP_LOGI("DFPlayer", "Attempting to play track %d (attempt %d)...", test_track, i + 1);
        dfplayer_play_track(test_track);
        vTaskDelay(pdMS_TO_TICKS(DFPLAYER_RETRY_DELAY_MS));
    }

    ESP_LOGI("DFPlayer", "DFPlayer Safe Init Complete.");
}
/**
 * @brief Initialize UART
 *
 */
void init_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_driver_install(UART_NUM, 1024, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Wait for DFPlayer to respond
    vTaskDelay(pdMS_TO_TICKS(300));
}
/**
 * @brief Display the current value on the timer
 * appData.amDigits are displayed using the current RGB Color for the state
 */
void Timer_Display(void)
{

    led_strip_handle_t led_strip = appData.ahLEDStrip;
    rgb_t RGBOn = getRGB();
    int nLed = 0;
    for (int nDigit = 0; nDigit < DISPLAY_DIGITS; nDigit++)
    {
        int nMask = SEG_A;
        int mThisDigit = appData.amDigits[nDigit];
        for (int nSegment = 0; nSegment < DIGIT_SEGMENTS; nSegment++)
        {
            rgb_t color = RGB_BLACK;
            if (mThisDigit & nMask)
            {
                color = RGBOn;
            }
            int nSegmentLeds = BASE_LEDS_PER_SEGMENT;
            if (nSegment == (DIGIT_SEGMENTS - 1))
            {
                nSegmentLeds = 1;
            }
            // ESP_LOGI(TAG, "Digit:%d Segment:%d Display:%08x Mask: %08x Color(%d,%d,%d)", nDigit, nSegment, mThisDigit, nMask, (int)RGB_GET_R(color), (int)RGB_GET_G(color), (int)RGB_GET_B(color));
            while (nSegmentLeds-- > 0)
            {
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, nLed++, RGB_GET_R(color), RGB_GET_G(color), RGB_GET_B(color)));
            }
            nMask <<= 1;
        }
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}
/**
 * @brief Initialize all the hardware
 *
 */
void HW_Initialize(void)
{
    ESP_LOGI(TAG, "Requesting strip with %d LEDS", LED_STRIP_TOTAL_LEDS);
    appData.ahLEDStrip = configure_led(LED_STRIP_PORT);

    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL << PUSH_BUTTON_PORT;
    // disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
    init_uart();
    dfplayer_safe_init(30, TRACK_WELCOME_TO_CODEBUSTERS);
}
/**
 * @brief Initialize all the application data and start the timer
 *
 */
void APP_Initialize(void)
{
    TimerHandle_t hTickTimer;
    appData.nPressedCount = 0;
    appData.nReleasedCount = 0;

    appData.hTimerSemaphore = xSemaphoreCreateBinary();
    appData.bStartState = true;
    appData.tStartTime = 0;
    appData.dLastSeconds = 0;

    for (int nDigit = 0; nDigit < DISPLAY_DIGITS; nDigit++)
    {
        appData.amDigits[nDigit] = SEG_ALL;
    }

    ESP_LOGI(TAG, "Timer Interval is %ld", TIMER_INTERVAL);
    hTickTimer = xTimerCreate("Tick", TIMER_INTERVAL, pdTRUE, (void *)2, &Process_Tick);
    // Check if the timer was created successfully
    if (hTickTimer == NULL)
    {
        ESP_LOGI(TAG, "Failed to create timer");
    }
    else
    {
        // Start the timers
        if (xTimerStart(hTickTimer, 0) != pdPASS)
        {
            ESP_LOGI(TAG, "Failed to start timer\n");
        }
    }
    Switch_To_State(APP_STATE_CODEBUSTERS);
}
/**
 * @brief Scroll the Codebusters text at the rate of 2/second
 *
 */
void ScrollCodebusters(void)
{
    const char *scrollMessage = "COdEbuST^ERS  ";

    if (appData.bStartState)
    {
        appData.tStartTime = esp_timer_get_time();
        appData.dLastSeconds = -1;
        appData.bStartState = false;
        dfplayer_play_track(TRACK_WELCOME_TO_CODEBUSTERS);
    }
    double elapsed_ticks = 2 * appData.dElapsedSeconds;
    int slot = ((int)(round(elapsed_ticks)) - 1 + strlen(scrollMessage)) % strlen(scrollMessage);
    if (slot != appData.dLastSeconds)
    {
        appData.dLastSeconds = slot;
        int nextSlot = (slot + 1) % strlen(scrollMessage);
        appData.amDigits[0] = Get_Segment_Mask(scrollMessage[slot]);
        appData.amDigits[1] = Get_Segment_Mask(scrollMessage[nextSlot]);
        Timer_Display();
    }
}
/**
 * @brief Show the countdown to zero in 10th of a second
 *
 */
void showSecondsCountdownTime(void)
{
    double dElapsedSecondsTenths = roundf((appData.tNow - appData.tStartTime) / 100000.0);

    int nTenthsRemain = ceil(((float)EVENT_LENGTH * 10) - dElapsedSecondsTenths);
    if (nTenthsRemain != appData.dLastSeconds)
    {
        appData.dLastSeconds = nTenthsRemain;

        int nSeconds = trunc(nTenthsRemain / 10);
        int nTenths = nTenthsRemain % 10;

        appData.amDigits[0] = Get_Segment_Mask(nSeconds) | SEG_DOT;
        appData.amDigits[1] = Get_Segment_Mask(nTenths);
        Timer_Display();
    }
}
/**
 * @brief Show the remaining time in minutes
 *
 */
void showCountdownTime(void)
{
    if (floor(appData.dElapsedSeconds) != floor(appData.dLastSeconds))
    {
        int nSecondsRemain;
        int nMinutesRemain;
        appData.dLastSeconds = appData.dElapsedSeconds;
        nSecondsRemain = ceil(EVENT_LENGTH - appData.dElapsedSeconds);
        nMinutesRemain = (((nSecondsRemain * SCALE_SPEED) + 59) / 60);

        appData.amDigits[0] = Get_Segment_Mask((nMinutesRemain % 100) / 10);
        appData.amDigits[1] = Get_Segment_Mask(nMinutesRemain % 10);
        ESP_LOGI(TAG, "Remain: %02d:%02d Time: %.2f", nMinutesRemain, nSecondsRemain % 60, appData.dElapsedSeconds);
        Timer_Display();
    }
}
/**
 * @brief Switch the state that the application is in
 *
 * @param newState New state to switch to
 */
void Switch_To_State(APP_STATES newState)
{
    appData.bStartState = true;
    appData.stateApp = newState;
}
/**
 * @brief Process a timed state transition
 *
 * @param timeLimit How long to stay in this state
 * @param track Track to play if the state transitions
 * @param nextState New state to transition
 * @return true State transitioned
 * @return false State did not transition
 */
bool HandleTimedState(double timeLimit, int track, APP_STATES nextState)
{
    if (appData.dElapsedSeconds >= timeLimit)
    {
        if (track != -1)
        {
            dfplayer_play_track(track);
        }
        Switch_To_State(nextState);
        return true;
    }
    showCountdownTime();
    return false;
}
/**
 * @brief Main application loop
 *
 */
void APP_Main(void)
{
    HW_Initialize();
    APP_Initialize();
    ESP_LOGI(TAG, "Initialized");

    appData.tStartTime = esp_timer_get_time(); // Record start time in microseconds
    appData.dLastSeconds = 0;

    for (;;)
    {

        // Wait for the timer to tell us to run another step.  Note that we will
        // timeout after double the expected time just to keep us running.
        xSemaphoreTake(appData.hTimerSemaphore, 2 * TIMER_INTERVAL);
        appData.tNow = esp_timer_get_time();
        // Compute the elapsed time to the nearest 10th of a second.
        appData.dElapsedSeconds = roundf((float)(appData.tNow - appData.tStartTime) / 100000.0) / 10.0;

        switch (appData.stateApp)
        {
        case APP_STATE_CODEBUSTERS:
            ScrollCodebusters();
            break;
        case APP_STATE_WAIT_START:
            appData.amDigits[0] = Get_Segment_Mask(5);
            appData.amDigits[1] = Get_Segment_Mask(0);
            Timer_Display();
            break;
        case APP_STATE_TIMED_QUESTION:

            if (appData.bStartState)
            {
                appData.tStartTime = appData.tNow;
                appData.dElapsedSeconds = 0;
                appData.bStartState = false;
            }
            HandleTimedState(END_TIMED_SECONDS, TRACK_NO_MORE_TIMED_BONUS, APP_STATE_WAIT_25_MINUTES);
            break;
        case APP_STATE_WAIT_25_MINUTES:
            HandleTimedState(ANNOUNCE_25_MINUTES, TRACK_25_MINUTES_REMAIN, APP_STATE_WAIT_10_MINUTES);
            break;
        case APP_STATE_WAIT_10_MINUTES:
            HandleTimedState(ANNOUNCE_10_MINUTES, TRACK_10_MINUTES_REMAIN, APP_STATE_WAIT_2_MINUTES);
            break;
        case APP_STATE_WAIT_2_MINUTES:
            HandleTimedState(ANNOUNCE_2_MINUTES, TRACK_2_MINUTES_REMAIN, APP_STATE_WAIT_10_SECONDS);
            break;
        case APP_STATE_WAIT_10_SECONDS:
            HandleTimedState(FINAL_SECONDS, -1, APP_STATE_FINAL_10SECONDS);
            break;

        case APP_STATE_FINAL_10SECONDS:
            if (appData.dElapsedSeconds >= EVENT_LENGTH)
            {
                dfplayer_play_track(TRACK_TIMES_UP);
                Switch_To_State(APP_STATE_DONE);
            }
            showSecondsCountdownTime();
            break;
        case APP_STATE_DONE:
            if (appData.bStartState)
            {
                appData.tStartTime = appData.tNow;
                appData.dElapsedSeconds = 0;
                appData.bStartState = false;

                appData.amDigits[0] = Get_Segment_Mask(0);
                appData.amDigits[1] = Get_Segment_Mask(0);
                Timer_Display();
            }
            break;
        case APP_STATE_CONFIG:
            break;
        }
    }
}
