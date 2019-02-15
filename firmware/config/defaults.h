/*

DEFAULT CONFIGURATION

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// =============================================================================
// Hardware default values
// =============================================================================

#define GPIO_NONE           0x99

// =============================================================================
// Buttons
// =============================================================================

#ifndef BUTTON1_PIN
#define BUTTON1_PIN         GPIO_NONE
#endif
#ifndef BUTTON2_PIN
#define BUTTON2_PIN         GPIO_NONE
#endif
#ifndef BUTTON3_PIN
#define BUTTON3_PIN         GPIO_NONE
#endif
#ifndef BUTTON4_PIN
#define BUTTON4_PIN         GPIO_NONE
#endif
#ifndef BUTTON5_PIN
#define BUTTON5_PIN         GPIO_NONE
#endif
#ifndef BUTTON6_PIN
#define BUTTON6_PIN         GPIO_NONE
#endif
#ifndef BUTTON7_PIN
#define BUTTON7_PIN         GPIO_NONE
#endif
#ifndef BUTTON8_PIN
#define BUTTON8_PIN         GPIO_NONE
#endif

// =============================================================================
// LEDs
// =============================================================================

#ifndef LED1_PIN
#define LED1_PIN            GPIO_NONE
#endif
#ifndef LED2_PIN
#define LED2_PIN            GPIO_NONE
#endif
#ifndef LED3_PIN
#define LED3_PIN            GPIO_NONE
#endif
#ifndef LED4_PIN
#define LED4_PIN            GPIO_NONE
#endif
#ifndef LED5_PIN
#define LED5_PIN            GPIO_NONE
#endif
#ifndef LED6_PIN
#define LED6_PIN            GPIO_NONE
#endif
#ifndef LED7_PIN
#define LED7_PIN            GPIO_NONE
#endif
#ifndef LED8_PIN
#define LED8_PIN            GPIO_NONE
#endif

#ifndef LED1_MODE
#define LED1_MODE           LED_MODE_BUS
#endif
#ifndef LED2_MODE
#define LED2_MODE           LED_MODE_OFF
#endif
#ifndef LED3_MODE
#define LED3_MODE           LED_MODE_OFF
#endif
#ifndef LED4_MODE
#define LED4_MODE           LED_MODE_OFF
#endif
#ifndef LED5_MODE
#define LED5_MODE           LED_MODE_OFF
#endif
#ifndef LED6_MODE
#define LED6_MODE           LED_MODE_OFF
#endif
#ifndef LED7_MODE
#define LED7_MODE           LED_MODE_OFF
#endif
#ifndef LED8_MODE
#define LED8_MODE           LED_MODE_OFF
#endif

#ifndef LED1_PIN_INVERSE
#define LED1_PIN_INVERSE    0
#endif
#ifndef LED2_PIN_INVERSE
#define LED2_PIN_INVERSE    0
#endif
#ifndef LED3_PIN_INVERSE
#define LED3_PIN_INVERSE    0
#endif
#ifndef LED4_PIN_INVERSE
#define LED4_PIN_INVERSE    0
#endif
#ifndef LED5_PIN_INVERSE
#define LED5_PIN_INVERSE    0
#endif
#ifndef LED6_PIN_INVERSE
#define LED6_PIN_INVERSE    0
#endif
#ifndef LED7_PIN_INVERSE
#define LED7_PIN_INVERSE    0
#endif
#ifndef LED8_PIN_INVERSE
#define LED8_PIN_INVERSE    0
#endif

// =============================================================================
// Relays
// =============================================================================

#ifndef RELAY1_PIN
#define RELAY1_PIN          GPIO_NONE
#endif

#ifndef RELAY2_PIN
#define RELAY2_PIN          GPIO_NONE
#endif

#ifndef RELAY3_PIN
#define RELAY3_PIN          GPIO_NONE
#endif

#ifndef RELAY4_PIN
#define RELAY4_PIN          GPIO_NONE
#endif

#ifndef RELAY5_PIN
#define RELAY5_PIN          GPIO_NONE
#endif

#ifndef RELAY6_PIN
#define RELAY6_PIN          GPIO_NONE
#endif

#ifndef RELAY7_PIN
#define RELAY7_PIN          GPIO_NONE
#endif

#ifndef RELAY8_PIN
#define RELAY8_PIN          GPIO_NONE
#endif

// -----------------------------------------------------------------------------

#ifndef RELAY1_TYPE
#define RELAY1_TYPE         RELAY_TYPE_NORMAL
#endif

#ifndef RELAY2_TYPE
#define RELAY2_TYPE         RELAY_TYPE_NORMAL
#endif

#ifndef RELAY3_TYPE
#define RELAY3_TYPE         RELAY_TYPE_NORMAL
#endif

#ifndef RELAY4_TYPE
#define RELAY4_TYPE         RELAY_TYPE_NORMAL
#endif

#ifndef RELAY5_TYPE
#define RELAY5_TYPE         RELAY_TYPE_NORMAL
#endif

#ifndef RELAY6_TYPE
#define RELAY6_TYPE         RELAY_TYPE_NORMAL
#endif

#ifndef RELAY7_TYPE
#define RELAY7_TYPE         RELAY_TYPE_NORMAL
#endif

#ifndef RELAY8_TYPE
#define RELAY8_TYPE         RELAY_TYPE_NORMAL
#endif

// -----------------------------------------------------------------------------

#ifndef RELAY1_RESET_PIN
#define RELAY1_RESET_PIN    GPIO_NONE
#endif

#ifndef RELAY2_RESET_PIN
#define RELAY2_RESET_PIN    GPIO_NONE
#endif

#ifndef RELAY3_RESET_PIN
#define RELAY3_RESET_PIN    GPIO_NONE
#endif

#ifndef RELAY4_RESET_PIN
#define RELAY4_RESET_PIN    GPIO_NONE
#endif

#ifndef RELAY5_RESET_PIN
#define RELAY5_RESET_PIN    GPIO_NONE
#endif

#ifndef RELAY6_RESET_PIN
#define RELAY6_RESET_PIN    GPIO_NONE
#endif

#ifndef RELAY7_RESET_PIN
#define RELAY7_RESET_PIN    GPIO_NONE
#endif

#ifndef RELAY8_RESET_PIN
#define RELAY8_RESET_PIN    GPIO_NONE
#endif

// -----------------------------------------------------------------------------

#ifndef RELAY1_DELAY_ON
#define RELAY1_DELAY_ON     0
#endif

#ifndef RELAY2_DELAY_ON
#define RELAY2_DELAY_ON     0
#endif

#ifndef RELAY3_DELAY_ON
#define RELAY3_DELAY_ON     0
#endif

#ifndef RELAY4_DELAY_ON
#define RELAY4_DELAY_ON     0
#endif

#ifndef RELAY5_DELAY_ON
#define RELAY5_DELAY_ON     0
#endif

#ifndef RELAY6_DELAY_ON
#define RELAY6_DELAY_ON     0
#endif

#ifndef RELAY7_DELAY_ON
#define RELAY7_DELAY_ON     0
#endif

#ifndef RELAY8_DELAY_ON
#define RELAY8_DELAY_ON     0
#endif

// -----------------------------------------------------------------------------

#ifndef RELAY1_DELAY_OFF
#define RELAY1_DELAY_OFF    0
#endif

#ifndef RELAY2_DELAY_OFF
#define RELAY2_DELAY_OFF    0
#endif

#ifndef RELAY3_DELAY_OFF
#define RELAY3_DELAY_OFF    0
#endif

#ifndef RELAY4_DELAY_OFF
#define RELAY4_DELAY_OFF    0
#endif

#ifndef RELAY5_DELAY_OFF
#define RELAY5_DELAY_OFF    0
#endif

#ifndef RELAY6_DELAY_OFF
#define RELAY6_DELAY_OFF    0
#endif

#ifndef RELAY7_DELAY_OFF
#define RELAY7_DELAY_OFF    0
#endif

#ifndef RELAY8_DELAY_OFF
#define RELAY8_DELAY_OFF    0
#endif
