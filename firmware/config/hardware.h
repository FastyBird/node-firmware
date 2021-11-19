/*

SUPPORTED HARDWARE DEFINITIONS

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#if defined(FASTYBIRD_IO_TEST)

    // Info
    #define DEVICE_MANUFACTURER                         "FASTYBIRD"
    #define DEVICE_NAME                                 "8CH_BUTTONS"

    // LEDs
    #define LED_MAX_ITEMS                               2

    #define LED1_PIN                                    13
    #define LED1_PIN_INVERSE                            0
    #define LED2_PIN                                    12
    #define LED2_PIN_INVERSE                            0

    led_t led_module_items[LED_MAX_ITEMS] = {
        // Pin     Is pin inverted   LED mode      Initial timestamp
        {LED1_PIN, LED1_PIN_INVERSE, LED_MODE_BUS, 0},
        {LED2_PIN, LED2_PIN_INVERSE, LED_MODE_OFF, 0},
    };

    // Buttons
    #define BUTTON_MAX_ITEMS                            4

    #define BUTTON1_PIN                                 6
    #define BUTTON2_PIN                                 7
    #define BUTTON3_PIN                                 8
    #define BUTTON4_PIN                                 9

    button_t button_module_items[BUTTON_MAX_ITEMS] = {
        // Debounce instance                                                                                                  AI register address
        //                                                                                                                       Initial status(event)
        {new DebounceEvent(BUTTON1_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 0, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON2_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 1, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON3_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 2, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON4_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 3, BUTTON_EVENT_NONE},
    };

    // Digital outputs
    #define RELAY_PROVIDER                              RELAY_PROVIDER_RELAY
    #define RELAY_MAX_ITEMS                             4

    #define RELAY1_PIN                                  A1
    #define RELAY2_PIN                                  A2
    #define RELAY3_PIN                                  A3
    #define RELAY4_PIN                                  A4

    relay_t relay_module_items[RELAY_MAX_ITEMS] = {
        {RELAY1_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 0, FLASH_ADDRESS_RELAY_01, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY2_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 1, FLASH_ADDRESS_RELAY_02, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY3_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 2, FLASH_ADDRESS_RELAY_03, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY4_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 3, FLASH_ADDRESS_RELAY_04, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
    };

    // BUS
    #define COMMUNICATION_BUS_TX_PIN                    3
    #define COMMUNICATION_BUS_RX_PIN                    2

    #define COMMUNICATION_MAX_DI_REGISTER_SIZE          0
    #define COMMUNICATION_MAX_DO_REGISTER_SIZE          0
    #define COMMUNICATION_MAX_AI_REGISTER_SIZE          4
    #define COMMUNICATION_MAX_AO_REGISTER_SIZE          4
    
    #define COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE      0
    #define COMMUNICATION_MAX_DEVICE_ATTRIBUTES_SIZE    0
    #define COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE   4

    communication_digital_register_t communication_module_di_registers[COMMUNICATION_MAX_DI_REGISTER_SIZE];
    communication_digital_register_t communication_module_do_registers[COMMUNICATION_MAX_DO_REGISTER_SIZE];

    communication_analog_register_t communication_module_ai_registers[COMMUNICATION_MAX_AI_REGISTER_SIZE] = {
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
    };
    communication_analog_register_t communication_module_ao_registers[COMMUNICATION_MAX_AO_REGISTER_SIZE] = {
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
    };

    communication_settings_device_t communication_module_settings_device[COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE];
    communication_attributes_device_t communication_module_attributes_device[COMMUNICATION_MAX_DEVICE_ATTRIBUTES_SIZE];
    communication_settings_register_t communication_module_settings_registers[COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE] = {
        {"DblDelay", 0, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 2, {0, 0, 1, 94}},
        {"DblDelay", 1, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 2, {0, 0, 1, 94}},
        {"DblDelay", 2, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 2, {0, 0, 1, 94}},
        {"DblDelay", 3, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 2, {0, 0, 1, 94}},
    };

    // GENERAL
    #define SYSTEM_CONFIGURE_DEVICE_BUTTON              0
    #define SYSTEM_DEVICE_COMMUNICATION_LED             0
    #define SYSTEM_DEVICE_STATE_LED                     1
#endif

#if defined(FASTYBIRD_IO_TEST_ARM)

    // Info
    #define DEVICE_MANUFACTURER                         "FASTYBIRD"
    #define DEVICE_NAME                                 "8CH_BUTTONS"

    // LEDs
    #define LED_MAX_ITEMS                               2

    #define LED1_PIN                                    13
    #define LED1_PIN_INVERSE                            0
    #define LED2_PIN                                    12
    #define LED2_PIN_INVERSE                            0

    led_t led_module_items[LED_MAX_ITEMS] = {
        // Pin     Is pin inverted   LED mode      Initial timestamp
        {LED1_PIN, LED1_PIN_INVERSE, LED_MODE_BUS, 0},
        {LED2_PIN, LED2_PIN_INVERSE, LED_MODE_OFF, 0},
    };

    // Buttons
    #define BUTTON_MAX_ITEMS                            4

    #define BUTTON1_PIN                                 4
    #define BUTTON2_PIN                                 5
    #define BUTTON3_PIN                                 6
    #define BUTTON4_PIN                                 7

    button_t button_module_items[BUTTON_MAX_ITEMS] = {
        // Debounce instance                                                                                                  AI register address
        //                                                                                                                       Initial status(event)
        {new DebounceEvent(BUTTON1_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 0, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON2_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 1, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON3_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 2, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON4_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 3, BUTTON_EVENT_NONE},
    };

    // Digital outputs
    #define RELAY_PROVIDER                              RELAY_PROVIDER_RELAY
    #define RELAY_MAX_ITEMS                             4

    #define RELAY1_PIN                                  A1
    #define RELAY2_PIN                                  A2
    #define RELAY3_PIN                                  A3
    #define RELAY4_PIN                                  A4

    relay_t relay_module_items[RELAY_MAX_ITEMS] = {
        {RELAY1_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 0, FLASH_ADDRESS_RELAY_01, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY2_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 1, FLASH_ADDRESS_RELAY_02, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY3_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 2, FLASH_ADDRESS_RELAY_03, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY4_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 3, FLASH_ADDRESS_RELAY_04, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
    };

    // BUS
    #define COMMUNICATION_BUS_HARDWARE_SERIAL           1

    #define COMMUNICATION_MAX_DI_REGISTER_SIZE          0
    #define COMMUNICATION_MAX_DO_REGISTER_SIZE          0
    #define COMMUNICATION_MAX_AI_REGISTER_SIZE          4
    #define COMMUNICATION_MAX_AO_REGISTER_SIZE          4
    
    #define COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE      0
    #define COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE   4

    #define COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS     4

    communication_digital_register_t communication_module_di_registers[COMMUNICATION_MAX_DI_REGISTER_SIZE];
    communication_digital_register_t communication_module_do_registers[COMMUNICATION_MAX_DO_REGISTER_SIZE];

    communication_analog_register_t communication_module_ai_registers[COMMUNICATION_MAX_AI_REGISTER_SIZE] = {
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
    };
    communication_analog_register_t communication_module_ao_registers[COMMUNICATION_MAX_AO_REGISTER_SIZE] = {
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
    };

    communication_settings_device_t communication_module_settings_device[COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE];
    communication_settings_register_t communication_module_settings_registers[COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE] = {
        {"DblDelay", 0, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 2, {0, 0, 1, 94}},
        {"DblDelay", 1, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 2, {0, 0, 1, 94}},
        {"DblDelay", 2, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 2, {0, 0, 1, 94}},
        {"DblDelay", 3, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 2, {0, 0, 1, 94}},
    };

    // GENERAL
    #define SYSTEM_CONFIGURE_DEVICE_BUTTON              0
    #define SYSTEM_DEVICE_COMMUNICATION_LED             0
    #define SYSTEM_DEVICE_STATE_LED                     1
#endif

#if defined(FASTYBIRD_8CH_BUTTONS) || defined(FASTYBIRD_16CH_BUTTONS)

    // Info
    #define DEVICE_MANUFACTURER                         "FASTYBIRD"
    #if defined(FASTYBIRD_8CH_BUTTONS)
        #define DEVICE_NAME                             "8CH_BUTTONS"
    #else
        #define DEVICE_NAME                             "16CH_BUTTONS"
    #endif

    // LEDs
    #define LED_MAX_ITEMS                               2

    #define LED1_PIN                                    13
    #define LED1_PIN_INVERSE                            0
    #define LED2_PIN                                    12
    #define LED2_PIN_INVERSE                            0

    led_t led_module_items[LED_MAX_ITEMS] = {
        // Pin     Is pin inverted   LED mode      Initial timestamp
        {LED1_PIN, LED1_PIN_INVERSE, LED_MODE_BUS, 0},
        {LED2_PIN, LED2_PIN_INVERSE, LED_MODE_OFF, 0},
    };

    // Buttons
    #if defined(FASTYBIRD_8CH_BUTTONS)
        #define BUTTON_MAX_ITEMS                        8
    #else
        #define BUTTON_MAX_ITEMS                        16
    #endif

    #define BUTTON1_PIN                                 4
    #define BUTTON2_PIN                                 5
    #define BUTTON3_PIN                                 6
    #define BUTTON4_PIN                                 7
    #define BUTTON5_PIN                                 8
    #define BUTTON6_PIN                                 9
    #define BUTTON7_PIN                                 10
    #define BUTTON8_PIN                                 11
    #if defined(FASTYBIRD_16CH_BUTTONS)
        #define BUTTON9_PIN                             12
        #define BUTTON10_PIN                            A0
        #define BUTTON11_PIN                            A1
        #define BUTTON12_PIN                            A2
        #define BUTTON13_PIN                            A3
        #define BUTTON14_PIN                            A4
        #define BUTTON15_PIN                            A5
        #define BUTTON16_PIN                            A6
    #endif

    button_t button_module_items[BUTTON_MAX_ITEMS] = {
        // Debounce instance                                                                                                  AI register address
        //                                                                                                                       Initial status(event)
        {new DebounceEvent(BUTTON1_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 0, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON2_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 1, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON3_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 2, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON4_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 3, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON5_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 4, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON6_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 5, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON7_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 6, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON8_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 7, BUTTON_EVENT_NONE},

        #if defined(FASTYBIRD_16CH_BUTTONS)
        {new DebounceEvent(BUTTON9_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 8, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON10_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 9, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON11_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 10, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON12_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 11, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON13_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 12, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON14_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 13, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON15_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 14, BUTTON_EVENT_NONE},
        {new DebounceEvent(BUTTON16_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), 15, BUTTON_EVENT_NONE},
        #endif
    };

    // BUS
    #define COMMUNICATION_BUS_TX_PIN                        3
    #define COMMUNICATION_BUS_RX_PIN                        2

    #define COMMUNICATION_MAX_DI_REGISTER_SIZE              0
    #define COMMUNICATION_MAX_DO_REGISTER_SIZE              0
    #if defined(FASTYBIRD_8CH_BUTTONS)
        #define COMMUNICATION_MAX_AI_REGISTER_SIZE          8
    #else
        #define COMMUNICATION_MAX_AI_REGISTER_SIZE          16
    #endif
    #define COMMUNICATION_MAX_AO_REGISTER_SIZE              0

    #define COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE          0
    #if defined(FASTYBIRD_8CH_BUTTONS)
        #define COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE   8
    #else
        #define COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE   16
    #endif

    communication_digital_register_t communication_module_di_registers[COMMUNICATION_MAX_DI_REGISTER_SIZE];
    communication_digital_register_t communication_module_do_registers[COMMUNICATION_MAX_DO_REGISTER_SIZE];

    communication_analog_register_t communication_module_ai_registers[COMMUNICATION_MAX_AI_REGISTER_SIZE] = {
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},

        #if defined(FASTYBIRD_16CH_BUTTONS)
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_UINT8, true, 1, {0, 0, 0, 0}, ""},
        #endif
    };
    communication_analog_register_t communication_module_ao_registers[COMMUNICATION_MAX_AO_REGISTER_SIZE];

    communication_settings_device_t communication_module_settings_device[COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE];
    communication_settings_register_t communication_module_settings_registers[COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE] = {
        {"DblDelay", 0, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 1, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 2, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 3, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 4, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 5, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 6, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 7, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},

        #if defined(FASTYBIRD_16CH_BUTTONS)
        {"DblDelay", 8, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 9, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 10, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 11, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 12, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 13, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 14, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        {"DblDelay", 15, COMMUNICATION_REGISTER_TYPE_AI, COMMUNICATION_DATA_TYPE_UINT16, 1, {0, 0, 1, 94}},
        #endif
    };

    // GENERAL
    #define SYSTEM_CONFIGURE_DEVICE_BUTTON              0
    #define SYSTEM_DEVICE_COMMUNICATION_LED             0
    #define SYSTEM_DEVICE_STATE_LED                     1
#endif

#if defined(FASTYBIRD_8CH_DO) || defined(FASTYBIRD_16CH_DO)

    // Info
    #define DEVICE_MANUFACTURER                         "FASTYBIRD"
    #if defined(FASTYBIRD_8CH_DO)
        #define DEVICE_NAME                             "8CH_DO"
    #else
        #define DEVICE_NAME                             "16CH_DO"
    #endif

    // LEDs
    #define LED_MAX_ITEMS                               2

    #define LED1_PIN                                    13
    #define LED1_PIN_INVERSE                            0
    #define LED2_PIN                                    12
    #define LED2_PIN_INVERSE                            0

    led_t led_module_items[LED_MAX_ITEMS] = {
        // Pin     Is pin inverted   LED mode      Initial timestamp
        {LED1_PIN, LED1_PIN_INVERSE, LED_MODE_BUS, 0},
        {LED2_PIN, LED2_PIN_INVERSE, LED_MODE_OFF, 0},
    };

    // Buttons
    #define BUTTON_MAX_ITEMS                            1

    #define BUTTON1_PIN                                 4

    button_t button_module_items[BUTTON_MAX_ITEMS] = {
        // Debounce instance
        //                                                                                                                                Initial status(event)
        {new DebounceEvent(BUTTON1_PIN, BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP, BUTTON_DEBOUNCE_DELAY, BUTTON_DBLCLICK_DELAY), INDEX_NONE, BUTTON_EVENT_NONE},
    };

    // Digital outputs
    #define RELAY_PROVIDER                              RELAY_PROVIDER_RELAY
    #if defined(FASTYBIRD_8CH_DO)
        #define RELAY_MAX_ITEMS                         8
    #else
        #define RELAY_MAX_ITEMS                         16
    #endif

    #define RELAY1_PIN                                  4
    #define RELAY2_PIN                                  5
    #define RELAY3_PIN                                  6
    #define RELAY4_PIN                                  7
    #define RELAY5_PIN                                  8
    #define RELAY6_PIN                                  9
    #define RELAY7_PIN                                  10
    #define RELAY8_PIN                                  11
    #if defined(FASTYBIRD_16CH_DO)
        #define RELAY9_PIN                              12
        #define RELAY10_PIN                             A0
        #define RELAY11_PIN                             A1
        #define RELAY12_PIN                             A2
        #define RELAY13_PIN                             A3
        #define RELAY14_PIN                             A4
        #define RELAY15_PIN                             A5
        #define RELAY16_PIN                             A6
    #endif

    relay_t relay_module_items[RELAY_MAX_ITEMS] = {
        {RELAY1_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 0, FLASH_ADDRESS_RELAY_01, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY2_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 1, FLASH_ADDRESS_RELAY_02, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY3_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 2, FLASH_ADDRESS_RELAY_03, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY4_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 3, FLASH_ADDRESS_RELAY_04, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY5_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 4, FLASH_ADDRESS_RELAY_05, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY6_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 5, FLASH_ADDRESS_RELAY_06, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY7_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 6, FLASH_ADDRESS_RELAY_07, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY8_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 7, FLASH_ADDRESS_RELAY_08, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},

        #if defined(FASTYBIRD_16CH_DO)
        {RELAY9_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 8, FLASH_ADDRESS_RELAY_09, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY10_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 9, FLASH_ADDRESS_RELAY_10, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY11_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 10, FLASH_ADDRESS_RELAY_11, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY12_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 11, FLASH_ADDRESS_RELAY_12, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY13_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 12, FLASH_ADDRESS_RELAY_13, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY14_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 13, FLASH_ADDRESS_RELAY_14, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY15_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 14, FLASH_ADDRESS_RELAY_15, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        {RELAY16_PIN, RELAY_TYPE_NORMAL, GPIO_NONE, 15, FLASH_ADDRESS_RELAY_16, RELAY_DELAY_ON, RELAY_DELAY_OFF, false, false, 0, 0, 0},
        #endif
    };

    // BUS
    #define COMMUNICATION_BUS_TX_PIN                        3
    #define COMMUNICATION_BUS_RX_PIN                        2

    #define COMMUNICATION_MAX_DI_REGISTER_SIZE              0
    #define COMMUNICATION_MAX_DO_REGISTER_SIZE              0
    #define COMMUNICATION_MAX_AI_REGISTER_SIZE              0
    #if defined(FASTYBIRD_8CH_DO)
        #define COMMUNICATION_MAX_AO_REGISTER_SIZE          8
    #else
        #define COMMUNICATION_MAX_AO_REGISTER_SIZE          16
    #endif

    #define COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE          0
    #define COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE       0

    #if defined(FASTYBIRD_8CH_DO)
        #define COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS     8
    #else
        #define COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS     16
    #endif

    communication_digital_register_t communication_module_di_registers[COMMUNICATION_MAX_DI_REGISTER_SIZE];
    communication_digital_register_t communication_module_do_registers[COMMUNICATION_MAX_DO_REGISTER_SIZE];

    communication_analog_register_t communication_module_ai_registers[COMMUNICATION_MAX_AI_REGISTER_SIZE];
    communication_analog_register_t communication_module_ao_registers[COMMUNICATION_MAX_AO_REGISTER_SIZE] = {
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},

        #if defined(FASTYBIRD_16CH_DO)
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        {COMMUNICATION_DATA_TYPE_BOOLEAN, true, 2, {0, 0, 0, 0}, ""},
        #endif
    };

    communication_settings_device_t communication_module_settings_device[COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE];
    communication_settings_register_t communication_module_settings_registers[COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE];

    // GENERAL
    #define SYSTEM_CONFIGURE_DEVICE_BUTTON              0
    #define SYSTEM_DEVICE_COMMUNICATION_LED             0
    #define SYSTEM_DEVICE_STATE_LED                     1
#endif

#if defined(FASTYBIRD_16CH_BUTTONS_EXPANDER)

    // Info
    #define DEVICE_MANUFACTURER         "FASTYBIRD"
    #define DEVICE_NAME                 "16CH_BUTTONS_EXPANDER"

    // BUS
    #define COMMUNICATION_BUS_TX_PIN    3
    #define COMMUNICATION_BUS_RX_PIN    2

    // LEDs
    #define LED1_PIN                    13
    #define LED1_PIN_INVERSE            0

    // Buttons
    #define BUTTON_EXPANDER_SUPPORT     1

#endif