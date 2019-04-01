/*

SUPPORTED HARDWARE DEFINITIONS

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#if defined(FASTYBIRD_IO_TEST)

    // Info
    #define NODE_MANUFACTURER           "FASTYBIRD"
    #define NODE_NAME                   "8CH_BUTTONS"

    // BUS
    #define COMMUNICATION_BUS_TX_PIN    3
    #define COMMUNICATION_BUS_RX_PIN    2

    // LEDs
    #define LED1_PIN                    A0
    #define LED1_PIN_INVERSE            0

    // Buttons
    #define BUTTON_SUPPORT              1

    #define BUTTON1_PIN                 4
    #define BUTTON2_PIN                 5
    #define BUTTON3_PIN                 6
    #define BUTTON4_PIN                 7

    #define BUTTON1_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON2_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON3_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON4_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP

    // Digital outputs
    #define RELAY_PROVIDER              RELAY_PROVIDER_RELAY

    #define RELAY1_PIN                  A1
    #define RELAY2_PIN                  A2
    #define RELAY3_PIN                  A3
    #define RELAY4_PIN                  A4

    #define RELAY1_TYPE                 RELAY_TYPE_NORMAL
    #define RELAY2_TYPE                 RELAY_TYPE_NORMAL
    #define RELAY3_TYPE                 RELAY_TYPE_NORMAL
    #define RELAY4_TYPE                 RELAY_TYPE_NORMAL

#elif defined(FASTYBIRD_8CH_BUTTONS)

    // Info
    #define NODE_MANUFACTURER           "FASTYBIRD"
    #define NODE_NAME                   "8CH_BUTTONS"

    // BUS
    #define COMMUNICATION_BUS_TX_PIN    3
    #define COMMUNICATION_BUS_RX_PIN    2

    // LEDs
    #define LED1_PIN                    13
    #define LED1_PIN_INVERSE            0

    // Buttons
    #define BUTTON_SUPPORT              1

    #define BUTTON1_PIN                 5
    #define BUTTON2_PIN                 6
    #define BUTTON3_PIN                 7
    #define BUTTON4_PIN                 8
    #define BUTTON5_PIN                 9
    #define BUTTON6_PIN                 10
    #define BUTTON7_PIN                 11
    #define BUTTON8_PIN                 12

    #define BUTTON1_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON2_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON3_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON4_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON5_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON6_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON7_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP
    #define BUTTON8_MODE                BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP

#elif defined(FASTYBIRD_16CH_BUTTONS_EXPANDER)

    // Info
    #define NODE_MANUFACTURER           "FASTYBIRD"
    #define NODE_NAME                   "16CH_BUTTONS_EXPANDER"

    // BUS
    #define COMMUNICATION_BUS_TX_PIN    3
    #define COMMUNICATION_BUS_RX_PIN    2

    // LEDs
    #define LED1_PIN                    13
    #define LED1_PIN_INVERSE            0

    // Buttons
    #define BUTTON_EXPANDER_SUPPORT     1

#endif