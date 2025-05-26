#include "buzzer.h"

void init_buzzer_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM); // Configura o GPIO como PWM
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice_num, 125.0f);     // Define o divisor do clock para 1 MHz
    pwm_set_wrap(slice_num, 1000);        // Define o TOP para frequência de 1 kHz
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), 0); // Razão cíclica inicial
    pwm_set_enabled(slice_num, true);     // Habilita o PWM
}

void set_buzzer_tone(uint gpio, uint freq) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    uint top = 1000000 / freq;            // Calcula o TOP para a frequência desejada
    pwm_set_wrap(slice_num, top);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), top / 2); // 50% duty cycle
}

void stop_buzzer(uint gpio) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), 0); // Desliga o PWM
}

void play_alarm_critic(){
    set_buzzer_tone(BUZZER_A, C4);
    sleep_ms(1500);
    stop_buzzer(BUZZER_A);
}

void play_alarm_reset() {
    set_buzzer_tone(BUZZER_B, G4);
    sleep_ms(333);
    stop_buzzer(BUZZER_B);
    sleep_ms(333);
    set_buzzer_tone(BUZZER_B, G4);
    sleep_ms(333);
    stop_buzzer(BUZZER_B);
}