#include "SCREEN_TemperaturePk.h"
#include "SCREEN_ReadPanel.h"
#include "LCD_BasicGraphics.h"
#include "LCD_fonts_images.h"
#include "LCD_Hardware.h"
#include "touch.h"
#include <math.h>
#include <stdio.h>
#include <limits.h>

/* The existing SDRAM drawing buffer, presented by the existing LCD_Show(). */
extern uint32_t pLcd[];

#define PANEL_W 800
#define PANEL_H 480
#define PANEL_TOUCH 61000U
#define PANEL_WHITE 0xFFFFFFFFU
#define PANEL_INK 0xFF15345BU
#define PANEL_MUTED 0xFF657990U

static struct {
    int active, fonts[4], temperature, previousPk;
    int wasDown, armed, pressed, previousPressed, pulse;
    uint32_t sampleTick, demoTick;
} panel;

uint8_t lcdPanelPkDemo;
volatile int lcdPanelResult;
volatile int lcdPanelTemperature10 = 255;
int (*lcdPanelReadTemperature10)(void);

static uint32_t PanelMix(uint32_t a, uint32_t b, unsigned t)
{
    unsigned r = (((a >> 16) & 255) * (255-t) + ((b >> 16) & 255) * t + 127) / 255;
    unsigned g = (((a >> 8) & 255) * (255-t) + ((b >> 8) & 255) * t + 127) / 255;
    unsigned v = ((a & 255) * (255-t) + (b & 255) * t + 127) / 255;
    return 0xFF000000U | (r << 16) | (g << 8) | v;
}

static void PanelPixel(int x, int y, uint32_t color, unsigned alpha)
{
    if ((unsigned)x >= PANEL_W || (unsigned)y >= PANEL_H || !alpha) return;
    LCD_Buffer(PANEL_W, x, y, alpha == 255 ? color : PanelMix(pLcd[y*PANEL_W+x], color, alpha));
}

/* Rounded vertical gradient with antialiased corners and source-over alpha.
 * It preserves the existing blue background instead of painting corner boxes.
 */
static void PanelRound(int x, int y, int w, int h, int radius,
                       uint32_t top, uint32_t bottom, unsigned opacity)
{
    if (w <= 0 || h <= 0) return;
    if (radius > w/2) radius = w/2;
    if (radius > h/2) radius = h/2;
    for (int j = 0; j < h; ++j) {
        uint32_t color = PanelMix(top, bottom, h > 1 ? (unsigned)(j*255/(h-1)) : 0);
        for (int i = 0; i < w; ++i) {
            float dx = i < radius ? radius-i-0.5f : (i >= w-radius ? i-(w-radius)+0.5f : 0);
            float dy = j < radius ? radius-j-0.5f : (j >= h-radius ? j-(h-radius)+0.5f : 0);
            unsigned alpha = opacity;
            if (dx > 0 && dy > 0) {
                float coverage = radius + 0.5f - sqrtf(dx*dx+dy*dy);
                if (coverage <= 0) continue;
                if (coverage < 1) alpha = (unsigned)(opacity*coverage);
            }
            PanelPixel(x+i, y+j, color, alpha);
        }
    }
}

static void PanelText(int font, int x, int y, char *text, uint32_t color)
{
    /* bkColor=0 uses the existing framebuffer behind antialiased glyphs. */
    LCD_StrChangeColor(font, x, y, text, fullHight, 0, 0, color, 255, NoConstWidth);
}

static void PanelCentered(int font, int center, int y, char *text, uint32_t color)
{
    int width = LCD_GetWholeStrPxlWidth(font, text, 0, NoConstWidth);
    PanelText(font, center-width/2, y, text, color);
}

static void PanelBackground(void)
{
    for (int y = 0; y < PANEL_H; ++y)
        for (int x = 0; x < PANEL_W; ++x) {
            unsigned t = (unsigned)(x*56/799 + y*199/479);
            int d = (x-65)*(x-65) + (y-10)*(y-10);
            unsigned glow = d < 250000 ? (unsigned)((250000-d)*64/250000) : 0;
            uint32_t color = PanelMix(0xFF2386F4U, 0xFF061A4BU, t);
            LCD_Buffer(PANEL_W, x, y, PanelMix(color, 0xFF7AE4FFU, glow));
        }

    PanelRound(667, 333, 240, 240, 120, PANEL_WHITE, PANEL_WHITE, 7);
    PanelRound(692, 358, 190, 190, 95, PANEL_WHITE, PANEL_WHITE, 5);

    PanelRound(28, 24, 44, 44, 14, 0xFF72C8FFU, 0xFF2379E9U, 255);
    PanelRound(46, 32, 8, 22, 4, PANEL_WHITE, PANEL_WHITE, 255);
    PanelRound(42, 48, 16, 16, 8, PANEL_WHITE, PANEL_WHITE, 255);
    PanelRound(48, 37, 4, 18, 2, 0xFF1472DBU, 0xFF1472DBU, 255);

    PanelText(panel.fonts[1], 86, 22, "Temperatura", PANEL_WHITE);
    PanelText(panel.fonts[0], 87, 62, "Panel sterowania", 0xFFD7E9FFU);
    PanelRound(610, 28, 162, 36, 18, PANEL_WHITE, PANEL_WHITE, 30);
    PanelCentered(panel.fonts[0], 705, 34, "TRYB DEMO", PANEL_WHITE);

    PanelRound(25, 121, 420, 294, 26, 0xFF00143BU, 0xFF00143BU, 38);
    PanelRound(459, 121, 316, 294, 26, 0xFF00143BU, 0xFF00143BU, 38);
    PanelRound(28, 115, 414, 292, 24, PANEL_WHITE, PANEL_WHITE, 255);
    PanelRound(462, 115, 310, 292, 24, PANEL_WHITE, PANEL_WHITE, 255);

    PanelText(panel.fonts[0], 54, 140, "POMIAR TEMPERATURY", PANEL_MUTED);
    PanelCentered(panel.fonts[0], 235, 367, "Stopnie Celsjusza", PANEL_MUTED);
    PanelCentered(panel.fonts[0], 617, 140, "PRZEKAZNIK", PANEL_MUTED);

    for (int x = 28; x < 772; ++x) PanelPixel(x, 434, PANEL_WHITE, 42);
    PanelText(panel.fonts[0], 30, 448, "Odczyt co 500 ms", 0xFFC6DAF7U);
    PanelCentered(panel.fonts[0], 670, 448, "DOTYK  /  PK DEMO", 0xFFC6DAF7U);
}

static void PanelTemperature(void)
{
    char text[24];
    int value = panel.temperature;
    int valid = value >= -500 && value <= 1500;
    if (valid) {
        unsigned magnitude = (unsigned)(value < 0 ? -value : value);
        snprintf(text, sizeof(text), "%s%u,%u", value < 0 ? "-" : "", magnitude/10, magnitude%10);
    } else snprintf(text, sizeof(text), "--,-");

    LCD_Shape(48, 183, LCD_Rectangle, 374, 123, PANEL_WHITE, PANEL_WHITE, PANEL_WHITE);
    int width = LCD_GetWholeStrPxlWidth(panel.fonts[3], text, 0, NoConstWidth);
    int x = 235 - (width+47)/2;
    PanelText(panel.fonts[3], x, 192, text, PANEL_INK);
    PanelText(panel.fonts[2], x+width+18, 215, "C", 0xFF3B79C9U);
    PanelRound(x+width+4, 209, 9, 9, 4, 0xFF3B79C9U, 0xFF3B79C9U, 255);
    PanelRound(x+width+6, 211, 5, 5, 2, PANEL_WHITE, PANEL_WHITE, 255);

    PanelRound(72, 326, 328, 8, 4, 0xFFE7EFFCU, 0xFFE7EFFCU, 255);
    int bounded = value < 0 ? 0 : (value > 500 ? 500 : value);
    int fill = valid ? 328*bounded/500 : 0;
    PanelRound(72, 326, fill, 8, 4, 0xFF59BCFFU, 0xFF0B57D0U, 255);
    PanelRound(624, 42, 8, 8, 4, panel.pulse ? 0xFF91F1C0U : 0xFFE8F1FFU,
               panel.pulse ? 0xFF91F1C0U : 0xFFE8F1FFU, 255);
}

static void PanelButton(int pk)
{
    LCD_Shape(482, 165, LCD_Rectangle, 270, 238, PANEL_WHITE, PANEL_WHITE, PANEL_WHITE);
    uint32_t top = pk ? 0xFF54A5FFU : 0xFF7094C4U;
    uint32_t bottom = pk ? 0xFF0B57D0U : 0xFF355785U;
    if (panel.pressed) { top = PanelMix(top, 0xFF001E50U, 65); bottom = PanelMix(bottom, 0xFF001E50U, 65); }
    PanelRound(528, 168, 178, 178, 89, pk ? 0xFFDCEBFFU : 0xFFECF1F8U,
               pk ? 0xFFDCEBFFU : 0xFFECF1F8U, 255);
    PanelRound(537, 181, 160, 160, 80, 0xFF16365FU, 0xFF16365FU, 28);
    int shift = panel.pressed ? 2 : 0;
    PanelRound(537, 177+shift, 160, 160, 80, top, bottom, 255);
    int cx = 617, cy = 235+shift;
    for (int dy = -32; dy <= 32; ++dy)
        for (int dx = -32; dx <= 32; ++dx) {
            float d = sqrtf((float)(dx*dx+dy*dy));
            float coverage = fminf(32.0f-d, d-25.0f);
            if (dy < -13 && dx > -12 && dx < 12) continue;
            if (coverage > 0) PanelPixel(cx+dx, cy+dy, PANEL_WHITE,
                                          coverage >= 1 ? 255 : (unsigned)(coverage*255));
        }
    PanelRound(614, 196+shift, 6, 29, 3, PANEL_WHITE, PANEL_WHITE, 255);
    PanelCentered(panel.fonts[2], 617, 281+shift, "PK", PANEL_WHITE);
    PanelCentered(panel.fonts[1], 617, 347, pk ? "WLACZONY" : "WYLACZONY",
                  pk ? 0xFF0B57D0U : PANEL_MUTED);
    PanelCentered(panel.fonts[0], 617, 381, pk ? "Dotknij, aby wylaczyc" : "Dotknij, aby wlaczyc", PANEL_MUTED);
}

static int PanelInside(XY_Touch_Struct p)
{
    int dx = (int)p.x-617, dy = (int)p.y-257;
    return dx*dx+dy*dy < 80*80;
}

int LCD_TemperaturePkPanel(uint8_t mode, int (*readTemperature10)(void), uint8_t *pkDemo)
{
    if (mode == LCD_TEMP_EXIT) {
        LCD_TOUCH_DeleteSelectTouch(PANEL_TOUCH);
        panel.active = 0; panel.armed = panel.pressed = 0;
        return 0;
    }
    if (!pkDemo || mode > LCD_TEMP_EXIT) return -1;
    if (LCD_GetXSize() != PANEL_W || LCD_GetYSize() != PANEL_H) return -2;
    uint32_t now = HAL_GetTick();
    int redrawAll = mode == LCD_TEMP_ENTER || !panel.active;
    int changed = 0, redrawTemperature = 0;

    if (redrawAll) {
        /* Exclusive full-screen view, like the existing screen entry functions. */
        panel.active = 0;
        SCREEN_ResetAllParameters();
        LCD_TOUCH_DeleteAllSetTouch();
        const int sizes[4] = {FONT_16, FONT_24, FONT_28_bold, FONT_72};
        const int ids[4] = {fontID_14, fontID_15, fontID_16, fontID_17};
        for (int i = 0; i < 4; ++i) {
            panel.fonts[i] = LCD_LoadFont_ChangeColor(sizes[i], Arial, ids[i]);
            if (panel.fonts[i] < 0) return -3;
        }
        if (LCDTOUCH_Set(537, 177, 160, 160, ID_TOUCH_POINT, PANEL_TOUCH, pressRelease) < 0) return -4;
        panel.demoTick = now;
        panel.sampleTick = now-500U;
        panel.previousPk = -1; panel.previousPressed = -1;
        panel.wasDown = LCD_TOUCH_isPress() == press;
        panel.armed = panel.pressed = panel.pulse = 0;
        panel.active = 1;
        PanelBackground();
    }

    screenTouchState = LCD_TOUCH_GetTypeAndPosition(&screenTouchPos);
    int down = LCD_TOUCH_isPress() == press;
    int inside = PanelInside(LCD_TOUCH_GetPos());
    if (down && !panel.wasDown) panel.armed = inside;
    if (!down && panel.wasDown) {
        if (panel.armed && inside) { *pkDemo = !*pkDemo; changed = 1; }
        panel.armed = 0;
    }
    panel.wasDown = down;
    panel.pressed = down && panel.armed && inside;

    if ((uint32_t)(now-panel.sampleTick) >= 500U) {
        panel.sampleTick = now;
        if (readTemperature10) panel.temperature = readTemperature10();
        else {
            /* Explicit demo: 25.5 -> 26.5 -> 24.5 -> 25.5 C. */
            unsigned n = (uint32_t)(now-panel.demoTick)/500U % 40U;
            panel.temperature = n <= 10 ? 255+(int)n : (n <= 30 ? 275-(int)n : 215+(int)n);
        }
        lcdPanelTemperature10 = panel.temperature;
        panel.pulse = !panel.pulse;
        redrawTemperature = 1;
        PanelTemperature();
    }
    int redrawButton = redrawAll || panel.previousPk != !!*pkDemo || panel.previousPressed != panel.pressed;
    if (redrawButton) {
        PanelButton(!!*pkDemo);
        panel.previousPk = !!*pkDemo; panel.previousPressed = panel.pressed;
    }

    if (redrawAll) LCD_Show();
    else {
        if (redrawTemperature) {
            LCD_DisplayPart(0, 48, 183, 374, 155);
            LCD_DisplayPart(0, 624, 42, 8, 8);
        }
        if (redrawButton) LCD_DisplayPart(0, 482, 165, 270, 238);
    }
    return changed;
}
