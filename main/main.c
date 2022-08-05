#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_nvs.h"
#include "app_adc_dac.h"
#include "app_button.h"
#include "app_led.h"

#define MAXIMO 2
#define ARRAY 12

bool btn_level = 0;

void app_main(void)
{
    static esp_err_t main_err;

    static const uint8_t g_btn = GPIO_NUM_23;
    static const uint8_t g_led = GPIO_NUM_32;
    static uint8_t g_adc_pines[MAXIMO] = {ADC1_CHANNEL_6, ADC1_CHANNEL_7};
    static uint32_t g_adc_val[MAXIMO];
    static uint8_t g_dac_val[MAXIMO];
    static uint8_t bits_adc[MAXIMO];
    static uint32_t adc1_array[ARRAY], adc2_array[ARRAY];
    static int sum1 = 0, sum2 = 0, prom_ADC1, prom_ADC2;

    gpio_set_direction(GPIO_NUM_2,GPIO_MODE_OUTPUT);

    printf("Setup\n");

    main_err = app_nvs_init();
    if (main_err != ESP_OK)
    {
        /* Que acciones tomar si existe un problema?*/;
        // esp_restart();
    }

    main_err = app_button_init(g_btn);
    if (main_err != ESP_OK)
    {
        /* Que acciones tomar si existe un problema? */
    }

    main_err = app_led_init(g_led);
    if (main_err != ESP_OK)
    {
        /* Que acciones tomar si existe un problema?*/
    }

    main_err = app_adc_init(g_adc_pines);
    if (main_err != ESP_OK)
    {
        /* Que acciones tomar si existe un problema?*/
    }

    main_err = app_dac_init();
    if (main_err != ESP_OK)
    {
        /* Que acciones tomar si existe un problema?*/
    }

    main_err = app_nvs_load_adc(g_dac_val);
    printf("%d\n", main_err);

    main_err = app_adc_calib();
    if (main_err != ESP_OK)
    {
        /* Que acciones tomar si existe un problema?*/
    }

    while (1)
    {

        btn_level = gpio_get_level(GPIO_NUM_23);
        printf("Button State: %i \n", btn_level);

        if (btn_level == 1)
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            sum1 = 0;
            sum2 = 0;
            prom_ADC1 = 0;
            prom_ADC2 = 0;

            main_err = app_adc_calib();
            if (main_err != ESP_OK)
            {
                /* Que acciones tomar si existe un problema?*/
            }

            for (int i = 0; i < 12; i++)
            {
                main_err = app_adc_read_milli_voltage(g_adc_val);
                if (main_err != ESP_OK)
                {
                    /* Que acciones tomar si existe un problema?*/
                }
                adc1_array[i] = g_adc_val[0];
                adc2_array[i] = g_adc_val[1];
                sum1 += adc1_array[i];
                sum2 += adc2_array[i];

                printf("ADC1: %i \t ADC2: %i \n", adc1_array[i], adc2_array[i]);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                if (i<20)
                {
                    gpio_set_level(GPIO_NUM_2,1);
                }
                   
            }
            prom_ADC1 = sum1 / 12;
            prom_ADC2 = sum2 / 12;
            gpio_set_level(GPIO_NUM_2,0);
            

            printf("\n P_ADC1: %i \t P_ADC2: %i \n", prom_ADC1, prom_ADC2);

            bits_adc[0] = (prom_ADC1 * 256) / 3100;
            bits_adc[1] = (prom_ADC2 * 256) / 3100;

            if (g_adc_val[0] && g_adc_val[1] < 2500) // El rango de error es mayor entre 0 y 2.5 V, por eso la compensación es mayor.
            {
                // Compensación negativa para mejorar resoución:
                g_dac_val[0] = bits_adc[0] - 4;
                g_dac_val[1] = bits_adc[1] - 8;
            }

            else if (g_adc_val[0] && g_adc_val[1] > 2500)
            {
                // Compensación negativa para mejorar resoución: 
                g_dac_val[0] = bits_adc[0] - 4;
                g_dac_val[1] = bits_adc[1] - 7;
            }

            main_err = app_nvs_save_adc(g_dac_val);
            if (main_err != ESP_OK)
            {
                /* Que acciones tomar si existe un problema?*/
            }

            main_err = app_dac_write(g_dac_val[0], g_dac_val[1]);
            if (main_err != ESP_OK)
            {
                /* Que acciones tomar si existe un problema?*/
            }
        }
        else
        {
            main_err = app_dac_write(g_dac_val[0], g_dac_val[1]);
            if (main_err != ESP_OK)
            {
                /* Que acciones tomar si existe un problema?*/
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
