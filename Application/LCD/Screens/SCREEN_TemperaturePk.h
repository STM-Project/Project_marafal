#ifndef SCREEN_TEMPERATURE_PK_H
#define SCREEN_TEMPERATURE_PK_H
#include <stdint.h>

enum { LCD_TEMP_UPDATE = 0, LCD_TEMP_ENTER = 1, LCD_TEMP_EXIT = 2 };

/* One public screen function. Call UPDATE every 20 ms from the GUI task.
 * readTemperature10: sensor callback (255 = 25.5 C), sampled every 500 ms.
 * NULL callback selects an explicitly simulated temperature.
 * pkDemo is caller-owned RAM state; this module never writes relay GPIO.
 * Returns 1 when touch changed pkDemo, 0 otherwise;
 * -1 invalid arguments/mode, -2 resolution, -3 font loading, -4 touch setup.
 * ENTER takes exclusive ownership of the screen and resets screen resources.
 * Fonts required on SD: Arial 16, Arial 24, Arial 28 bold, Arial 72.
 */
int LCD_TemperaturePkPanel(uint8_t mode, int (*readTemperature10)(void), uint8_t *pkDemo);

/* Default screen 7 demonstration, editable in the debugger. */
extern uint8_t lcdPanelPkDemo;
extern volatile int lcdPanelResult;
extern volatile int lcdPanelTemperature10;
extern int (*lcdPanelReadTemperature10)(void);
#endif
