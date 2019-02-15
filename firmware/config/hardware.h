/*

SUPPORTED HARDWARE DEFINITIONS

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#if defined(FASTYBIRD_8CH_BUTTONS)

    // Info
    #define NODE_MANUFACTURER  "FASTYBIRD"
    #define NODE_NAME          "8CH_BUTTONS"

    #define NODE_BUTTON_MODE    BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP

    // Buttons
    #define BUTTON_SUPPORT      0

    #define BUTTON_PINS         {13, 12, 11, 10, 8, 7, 6, 5}
    #define BUTTON_MODES        {NODE_BUTTON_MODE, NODE_BUTTON_MODE, NODE_BUTTON_MODE, NODE_BUTTON_MODE, NODE_BUTTON_MODE, NODE_BUTTON_MODE, NODE_BUTTON_MODE, NODE_BUTTON_MODE}

    #define BUTTON1_PIN         4
    #define BUTTON2_PIN         5
    #define BUTTON3_PIN         6
    #define BUTTON4_PIN         7

    #define BUTTON1_MODE        NODE_BUTTON_MODE
    #define BUTTON2_MODE        NODE_BUTTON_MODE
    #define BUTTON3_MODE        NODE_BUTTON_MODE
    #define BUTTON4_MODE        NODE_BUTTON_MODE

    #define BUTTON1_DI_REGISTRY 1
    #define BUTTON2_DI_REGISTRY 2
    #define BUTTON3_DI_REGISTRY 3
    #define BUTTON4_DI_REGISTRY 4

    // LEDs
    #define LED1_PIN            A0
    #define LED1_PIN_INVERSE    0

    // Digital inputs
    #define DI_SUPPORT          0

    #define DI1_PIN             4
    #define DI2_PIN             5
    #define DI3_PIN             6
    #define DI4_PIN             7

    #define DI1_MODE            NODE_BUTTON_MODE
    #define DI2_MODE            NODE_BUTTON_MODE
    #define DI3_MODE            NODE_BUTTON_MODE
    #define DI4_MODE            NODE_BUTTON_MODE

    #define DI1_DI_REGISTRY     1
    #define DI2_DI_REGISTRY     2
    #define DI3_DI_REGISTRY     3
    #define DI4_DI_REGISTRY     4

    // Digital outputs
    #define RELAY_SUPPORT       1
    #define RELAY_PROVIDER      RELAY_PROVIDER_RELAY

    #define RELAY1_PIN          A1
    #define RELAY2_PIN          A2
    #define RELAY3_PIN          A3
    #define RELAY4_PIN          A4

    #define RELAY1_TYPE         RELAY_TYPE_NORMAL
    #define RELAY2_TYPE         RELAY_TYPE_NORMAL
    #define RELAY3_TYPE         RELAY_TYPE_NORMAL
    #define RELAY4_TYPE         RELAY_TYPE_NORMAL

    #define RELAY1_DO_REGISTRY  1
    #define RELAY2_DO_REGISTRY  2
    #define RELAY3_DO_REGISTRY  3
    #define RELAY4_DO_REGISTRY  4

#endif