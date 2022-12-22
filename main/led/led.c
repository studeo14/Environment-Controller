#include "led.h"

static uint8_t s_led_state = 0;
static const char *TAG = "LED";

static led_strip_handle_t led_strip;

void set_pixel(uint32_t red, uint32_t green, uint32_t blue){
    led_strip_set_pixel(led_strip, 0, red, green, blue);
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);
}

void blink_led(void) {
  /* If the addressable LED is enabled */
  if (s_led_state) {
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel(led_strip, 0, 0xFF, 0x0, 0x0);
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);
  } else {
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
  }
}

void configure_led(void) {
  /* setup and turn on the power GPIO for the led_strip */
  gpio_reset_pin(BLINK_POWER);
  gpio_set_direction(BLINK_POWER, GPIO_MODE_OUTPUT);
  gpio_set_level(BLINK_POWER, 1);
  //
  ESP_LOGI(TAG, "Example configured to blink addressable LED!");
  /* LED strip initialization with the GPIO and pixels number*/
  led_strip_config_t strip_config = {
      .strip_gpio_num = BLINK_GPIO,
      .max_leds = 1, // at least one LED on board
  };
  led_strip_rmt_config_t rmt_config = {
      .resolution_hz = 10 * 1000 * 1000, // 10MHz
  };
  ESP_ERROR_CHECK(
      led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
  /* Set all LED off to clear all pixels */
  led_strip_clear(led_strip);
}

rgb hsv2rgb(hsv HSV)
{
    rgb RGB;
    double H = HSV.h, S = HSV.s, V = HSV.v,
            P, Q, T,
            fract;

    (H == 360.)?(H = 0.):(H /= 60.);
    fract = H - floor(H);

    P = V*(1. - S);
    Q = V*(1. - S*fract);
    T = V*(1. - S*(1. - fract));

    if      (0. <= H && H < 1.)
        RGB = (rgb){.r = V, .g = T, .b = P};
    else if (1. <= H && H < 2.)
        RGB = (rgb){.r = Q, .g = V, .b = P};
    else if (2. <= H && H < 3.)
        RGB = (rgb){.r = P, .g = V, .b = T};
    else if (3. <= H && H < 4.)
        RGB = (rgb){.r = P, .g = Q, .b = V};
    else if (4. <= H && H < 5.)
        RGB = (rgb){.r = T, .g = P, .b = V};
    else if (5. <= H && H < 6.)
        RGB = (rgb){.r = V, .g = P, .b = Q};
    else
        RGB = (rgb){.r = 0., .g = 0., .b = 0.};

    return RGB;
}

void rainbow_task(void * pvParameters) {
  hsv hue = {
    .h = 0.0,
    .s = 0.9,
    .v = 0.5
  };
  while (1) {
    for (long firstPixelHue = 0; firstPixelHue < 360; firstPixelHue++) {
      hue.h = (double)firstPixelHue;
      rgb color = hsv2rgb(hue);
      set_pixel(color.r * 255.0, color.g * 255.0, color.b * 255.0);
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }
  }
}