#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "ssd1306.h"
#include "font.h"
#include "buzzer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define I2C_OLED_PORT i2c1
#define I2C_OLED_SDA 15
#define I2C_OLED_SCL 14

#define BUTTON_A 5
#define BUTTON_B 6
#define JOYSTICK_BUTTON 22

#define RED_LED 13
#define GREEN_LED 11
#define BLUE_LED 12

#define DEBOUNCE_TIME 200
#define MAX_USERS 10

ssd1306_t ssd;

SemaphoreHandle_t xUserCountSemaphore;
SemaphoreHandle_t xResetSemaphore;
SemaphoreHandle_t xDisplayMutex;
volatile uint32_t activeUsers = 0;

void vEntryTask(void *pvParameters);
void vExitTask(void *pvParameters);
void vResetTask(void *pvParameters);
void vDisplayTask(void *pvParameters);
void vLEDTask(void *pvParameters);

void setRGBColor(uint8_t r, uint8_t g, uint8_t b);
void initializeI2C(void);

void initializeI2C(void) {
    i2c_init(I2C_OLED_PORT, 400 * 1000);
    gpio_set_function(I2C_OLED_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_OLED_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_OLED_SDA);
    gpio_pull_up(I2C_OLED_SCL);
}

int main()
{
    stdio_init_all();
    
    // Inicialização dos botões
    gpio_init(BUTTON_A); 
    gpio_set_dir(BUTTON_A, GPIO_IN); 
    gpio_pull_up(BUTTON_A);
    
    gpio_init(BUTTON_B); 
    gpio_set_dir(BUTTON_B, GPIO_IN); 
    gpio_pull_up(BUTTON_B);
    
    gpio_init(JOYSTICK_BUTTON); 
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN); 
    gpio_pull_up(JOYSTICK_BUTTON);
    
    // Inicialização dos LEDs
    gpio_init(RED_LED); 
    gpio_set_dir(RED_LED, GPIO_OUT);
    gpio_put(RED_LED, 0);
    
    gpio_init(GREEN_LED); 
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_put(GREEN_LED, 0);
    
    gpio_init(BLUE_LED); 
    gpio_set_dir(BLUE_LED, GPIO_OUT);
    gpio_put(BLUE_LED, 0);

    // Inicialização do I2C e display
    initializeI2C();
    sleep_ms(100); // Aguarda estabilização
    
    ssd1306_init(&ssd, 128, 64, false, 0x3C, I2C_OLED_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    
    init_buzzer_pwm(BUZZER_A);
    init_buzzer_pwm(BUZZER_B);

    // Criação dos semáforos
    xUserCountSemaphore = xSemaphoreCreateCounting(MAX_USERS, MAX_USERS);
    xResetSemaphore = xSemaphoreCreateBinary();
    xDisplayMutex = xSemaphoreCreateMutex();

    // Verificação se os semáforos foram criados com sucesso
    if (xUserCountSemaphore == NULL || xResetSemaphore == NULL || xDisplayMutex == NULL) {
        // Erro na criação dos semáforos
        while(1) {
            gpio_put(RED_LED, 1);
            sleep_ms(100);
            gpio_put(RED_LED, 0);
            sleep_ms(100);
        }
    }

    // Criação das tarefas com verificação de erro
    BaseType_t xReturned;
    
    xReturned = xTaskCreate(vEntryTask, "Entry", 512, NULL, 2, NULL);
    if (xReturned != pdPASS) while(1);

    xReturned = xTaskCreate(vExitTask, "Exit", 512, NULL, 2, NULL);
    if (xReturned != pdPASS) while(1);

    xReturned = xTaskCreate(vResetTask, "Reset", 512, NULL, 2, NULL);
    if (xReturned != pdPASS) while(1);

    xReturned = xTaskCreate(vDisplayTask, "Display", 1024, NULL, 1, NULL);
    if (xReturned != pdPASS) while(1);

    xReturned = xTaskCreate(vLEDTask, "LED", 256, NULL, 1, NULL);
    if (xReturned != pdPASS) while(1);
    
    vTaskStartScheduler();
    
    // Nunca deve chegar aqui
    while (1) {
        gpio_put(RED_LED, 1);
        sleep_ms(500);
        gpio_put(RED_LED, 0);
        sleep_ms(500);
    }
}

void vEntryTask(void *pvParameters) {
    TickType_t lastPress = 0;
    
    while(1) {
        if (!gpio_get(BUTTON_A)) {
            TickType_t currentTime = xTaskGetTickCount();
            if ((currentTime - lastPress) > pdMS_TO_TICKS(DEBOUNCE_TIME)) {
                if (xSemaphoreTake(xUserCountSemaphore, pdMS_TO_TICKS(100))) {
                    if (xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(100))) {
                        if (activeUsers < MAX_USERS) {
                            activeUsers++;
                        } else {
                            // Se o sistema estiver lotado, toca um alarme
                            play_alarm_critic();
                        }
                        xSemaphoreGive(xDisplayMutex);
                    }
                    lastPress = currentTime;
                } else {
                    // Semáforo não disponível, devolve o contador
                    xSemaphoreGive(xUserCountSemaphore);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vExitTask(void *pvParameters) {
    TickType_t lastPress = 0;
    
    while(1) {
        if (!gpio_get(BUTTON_B)) {
            TickType_t currentTime = xTaskGetTickCount();
            if ((currentTime - lastPress) > pdMS_TO_TICKS(DEBOUNCE_TIME)) {
                if (xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(100))) {
                    if (activeUsers > 0) {
                        activeUsers--;
                        xSemaphoreGive(xUserCountSemaphore);
                    }
                    xSemaphoreGive(xDisplayMutex);
                    lastPress = currentTime;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vResetTask(void *pvParameters) {
    TickType_t lastPress = 0;
    
    while(1) {
        if (!gpio_get(JOYSTICK_BUTTON)) {
            TickType_t currentTime = xTaskGetTickCount();
            if ((currentTime - lastPress) > pdMS_TO_TICKS(DEBOUNCE_TIME)) {
                if (xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(100))) {
                    // Reseta todos os usuários
                    while (activeUsers > 0) {
                        xSemaphoreGive(xUserCountSemaphore);
                        activeUsers--;
                    }
                    play_alarm_reset();
                    xSemaphoreGive(xResetSemaphore);
                    xSemaphoreGive(xDisplayMutex);
                    lastPress = currentTime;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vDisplayTask(void *pvParameters) {
    bool displayInitialized = true;
    
    while(1) {
        if (xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(100))) {
            if (displayInitialized) {
                ssd1306_fill(&ssd, false);
                
                // Título
                ssd1306_draw_string(&ssd, "CONTROLE DE ACESSO", 0, 5);
                
                // Contagem de usuários
                char userStr[32];
                snprintf(userStr, sizeof(userStr), "Usuarios: %lu/%d", activeUsers, MAX_USERS);
                ssd1306_draw_string(&ssd, userStr, 0, 20);
                
                // Estado do sistema
                if (activeUsers == 0) {
                    ssd1306_draw_string(&ssd, "Sistema vazio", 0, 35);
                } else if (activeUsers >= MAX_USERS) {
                    ssd1306_draw_string(&ssd, "LOTADO!", 0, 35);
                } else {
                    ssd1306_draw_string(&ssd, "Acesso liberado", 0, 35);
                }
                
                // Verifica se houve reset
                if (xSemaphoreTake(xResetSemaphore, 0) == pdTRUE) {
                    ssd1306_draw_string(&ssd, "RESET REALIZADO!", 0, 50);
                }
                
                ssd1306_send_data(&ssd);
            }
            xSemaphoreGive(xDisplayMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vLEDTask(void *pvParameters) {
    while(1) {
        uint32_t currentUsers;
        
        // Lê o número atual de usuários de forma segura
        if (xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(50))) {
            currentUsers = activeUsers;
            xSemaphoreGive(xDisplayMutex);
        } else {
            currentUsers = 0;
        }
        
        int occupancy = currentUsers;
        
        if (occupancy == 0) {
            setRGBColor(0, 0, 1); // Azul (vazio)
        } else if (occupancy <= MAX_USERS - 2) {
            setRGBColor(0, 1, 0); // Verde (<50%)
        } else if (occupancy <= MAX_USERS - 1) {
            setRGBColor(1, 1, 0); // Amarelo (50-80%)
        } else {
            setRGBColor(1, 0, 0); // Vermelho (>80%)
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void setRGBColor(uint8_t r, uint8_t g, uint8_t b) {
    gpio_put(RED_LED, r);
    gpio_put(GREEN_LED, g);
    gpio_put(BLUE_LED, b);
}