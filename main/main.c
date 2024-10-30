/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// microphone e audio
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/irq.h"  // interrupts
#include "hardware/pwm.h"  // pwm 
#include "hardware/sync.h" // wait for interrupt 

// Microphone
#define ADC_NUM 0
#define ADC_PIN (26 + ADC_NUM)
#define ADC_VREF 3.3
#define ADC_RANGE (1 << 12)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

// Audio
#define AUDIO_PIN 28

// Botão
#define BUTTON_PIN 15

void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));    
}

int main() {
    stdio_init_all();
    //printf("Sistema inicializado. Capturando e reproduzindo áudio enquanto o botão está pressionado...\n");

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);  

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_NUM);

    set_sys_clock_khz(176000, true);  
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 2.0f);  
    pwm_config_set_wrap(&config, 255); // 250
    pwm_init(audio_pin_slice, &config, true);
    pwm_set_gpio_level(AUDIO_PIN, 0);  

    uint16_t adc_raw;

    while (1) {
        if (gpio_get(BUTTON_PIN) == 0) {
            adc_raw = adc_read(); 
            //printf("ADC Value: %d\n", adc_raw); 
            uint16_t pwm_value = adc_raw >>4;
            pwm_set_gpio_level(AUDIO_PIN, pwm_value); 
        } else {
            pwm_set_gpio_level(AUDIO_PIN, 0);
        }
        // sleep_ms(10);  
    }

    return 0;
}
