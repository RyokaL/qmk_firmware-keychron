RGB_MATRIX_EFFECT(FIRE_RGB)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define RGB_FIRE_RAND 1
#define RGB_FIRE_DECAY 10
#define RGB_FIRE_MAXV 255
#define RGB_FIRE_BASEV 128
#define RGB_FIRE_LOWV 64
#define RGB_FIRE_SAT 255
#define RGB_FIRE_VALDIF (RGB_FIRE_MAXV - RGB_FIRE_BASEV)

    typedef struct {
        uint8_t baseH;
        uint8_t highH;
    } ColourConfig;

    static ColourConfig get_colour_config(uint8_t uH) {
        const ColourConfig colourConfigs[] = {
            {1, 40}, //Red
            {30, 43}, //Gold
            {80, 110}, //Green
            {170, 140}, //Blue
            {190, 210}, //Purple
            {235, 215},  //Pink
            {1, 254} //Rainbow?
        };

        if(uH <= colourConfigs[1].baseH) {
            return colourConfigs[0];
        }
        else if(uH <= colourConfigs[1].highH) {
            return colourConfigs[1];
        }
        else if(uH <= colourConfigs[2].highH) {
            return colourConfigs[2];
        }
        else if(uH <= colourConfigs[3].baseH) {
            return colourConfigs[3];
        }
        else if(uH <= colourConfigs[4].highH) {
            return colourConfigs[4];
        }
        else if(uH <= colourConfigs[5].baseH) {
            return colourConfigs[5];
        }
        else {
            return colourConfigs[6];
        }
        
    }

    //Borrowed from https://getreuer.info/posts/keyboards/macros3/index.html#random-emojis
    static uint8_t simple_rand(void) {
        static uint16_t random = 1;
        random *= UINT16_C(36563);
        return (uint8_t)(random >> 8);
    }

    bool FIRE_RGB(effect_params_t* params) {
        RGB_MATRIX_USE_LIMITS(led_min, led_max);

        ColourConfig flameColour;
        flameColour = get_colour_config(rgb_matrix_config.hsv.h);
        static uint8_t decay = 0;

        if(params->init) {
            HSV initHSV = {flameColour.baseH, RGB_FIRE_SAT, RGB_FIRE_BASEV};
            RGB initRGB = rgb_matrix_hsv_to_rgb(initHSV);
            rgb_matrix_set_color_all(initRGB.r, initRGB.b, initRGB.g);
            memset(g_rgb_frame_buffer, RGB_FIRE_BASEV, sizeof(g_rgb_frame_buffer));
        }

        uint16_t time = scale16by8(g_rgb_timer, rgb_matrix_config.speed / 8);
        decay++;

        for(uint8_t col = 0; col < MATRIX_COLS; col++) {
            for(uint8_t row = 0; row < MATRIX_ROWS; row++) {
                uint8_t newHue = 0;
                uint8_t newVal = 0;

                uint8_t led[LED_HITS_TO_REMEMBER];
                uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, led);
                if(led_count == 0) {
                    continue;
                }

                if(g_rgb_frame_buffer[row][col] == RGB_FIRE_BASEV && simple_rand() < RGB_FIRE_RAND) {
                    g_rgb_frame_buffer[row][col] = ((RGB_FIRE_MAXV * simple_rand()) >> 8) + RGB_FIRE_BASEV;
                    newHue = flameColour.highH;
                    newVal = g_rgb_frame_buffer[row][col];
                }
                else if(g_rgb_frame_buffer[row][col] > RGB_FIRE_BASEV) { //decay colour and brightness back to base
                    if(decay >= RGB_FIRE_DECAY) g_rgb_frame_buffer[row][col]--;
                    //lerp from current hue closer to target base hue - this formula is close enough
                    if(flameColour.baseH > flameColour.highH) {
                        newHue = flameColour.highH + ((RGB_FIRE_MAXV - g_rgb_frame_buffer[row][col])/(RGB_FIRE_VALDIF/(flameColour.baseH - flameColour.highH)));
                        //Underflow checker
                        if(newHue >= 0 && newHue < flameColour.highH) {
                            newHue = flameColour.baseH;
                        }
                    }
                    else {
                        newHue = flameColour.highH - ((RGB_FIRE_MAXV - g_rgb_frame_buffer[row][col])/(RGB_FIRE_VALDIF/(flameColour.highH - flameColour.baseH)));
                        //Overflow checker
                        if(newHue <= 255 && newHue > flameColour.highH) {
                            newHue = flameColour.baseH;
                        }
                    }
                    newVal = g_rgb_frame_buffer[row][col];
                }
                else if(g_rgb_frame_buffer[row][col] <= RGB_FIRE_BASEV) {
                    g_rgb_frame_buffer[row][col] = RGB_FIRE_BASEV;
                    newVal = scale8(abs8(sin8(time) - 128) * 2, RGB_FIRE_BASEV - RGB_FIRE_LOWV) + RGB_FIRE_LOWV;
                    newHue = flameColour.baseH;
                }

                HSV newHsv = {newHue, RGB_FIRE_SAT, newVal};
                RGB rgb = rgb_matrix_hsv_to_rgb(newHsv);
                
                rgb_matrix_set_color(led[0], rgb.r, rgb.g, rgb.b);
            }
        }
        if(decay >= RGB_FIRE_DECAY) {
            decay = 0;
        }
        return rgb_matrix_check_finished_leds(led_max);
    }
#endif