//Programa por Diego Garcia, Controla el giro de un motor de pasos con un drv8825

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/ledc.h"  
#include "driver/adc.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "font8x8_basic.h" 

//Definimos Pin de Seteo
#define set 27
adc1_channel_t adc_pot = ADC1_CHANNEL_5;

//Definimos los pines para velocidad y direccion giro del motor 
#define PIN_PULS 2
#define direc 26

//Definimos el nombre del parametro PWM
ledc_channel_config_t pwm;

#define tag "SSD1306"


void app_main(){

//Definimos valores del adc /////////////////////////////////////////////////////////

adc1_config_width(ADC_WIDTH_BIT_10);
adc1_config_channel_atten(adc_pot, ADC_ATTEN_DB_11);

//Definimos Valores del pwm /////////////////////////////////////////////////////////

ledc_timer_config_t timer ={
        .duty_resolution=LEDC_TIMER_10_BIT,    
        .speed_mode=LEDC_HIGH_SPEED_MODE,
        .timer_num=LEDC_TIMER_0,
        .clk_cfg=LEDC_AUTO_CLK,
        };    
pwm.channel = LEDC_CHANNEL_0;
        pwm.duty = 0;
        pwm.gpio_num = PIN_PULS;
        pwm.hpoint = 0;
        pwm.timer_sel = LEDC_TIMER_0;
        pwm.speed_mode = LEDC_HIGH_SPEED_MODE;

        ledc_channel_config(&pwm);

//Definimos los parametros para la pantalla ///////////////////////////////////////////////////////////

SSD1306_t dev;

#if CONFIG_I2C_INTERFACE                                                                 
	ESP_LOGI(tag, "INTERFACE is i2c");
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE

#if CONFIG_SSD1306_128x32
	ESP_LOGI(tag, "Panel is 128x32");
	ssd1306_init(&dev, 128, 32);
#endif // CONFIG_SSD1306_128x32 


//Definimos las salidas////////////////////////////////////////////////////////////////////////////
gpio_set_direction(direc, GPIO_MODE_OUTPUT);

//Definimos las entradas
gpio_set_direction(set, GPIO_MODE_INPUT);

//Definimos la conf inicial de la pantalla
ssd1306_clear_screen(&dev, false);
ssd1306_contrast(&dev, 0xff);
ssd1306_display_text(&dev, 3, "Izquierda <====", 15, false);
//Variables de control////////////////////////////////////////////////////////////////
bool cam=0; //para cambiar el estado de la salida que controlara la direccion del motor 

while(1){

        if(gpio_get_level(set)==1){

                cam=!cam;
                gpio_set_level(direc,cam);
                ssd1306_clear_line(&dev, 3, false);
                ssd1306_contrast(&dev, 0xff);
                if(cam==0){
                        ssd1306_display_text(&dev, 3, "Izquierda <====", 15, false);
                }
                else{
                        ssd1306_display_text(&dev, 3, "Derecha ======>", 15, false);
                }
                while(gpio_get_level(set)==1);
        }

        int pot = adc1_get_raw(adc_pot);
        int puls=0;
        
                if (pot < 250){
                        puls=100;
                        ssd1306_display_text(&dev, 2, "       100      ", 16, false);
                }
                
                if (pot >= 250 && pot < 500){
                        ssd1306_display_text(&dev, 2, "       250      ", 16, false);
                        puls=250;
                }
                
                if (pot >= 500 && pot < 750){
                        ssd1306_display_text(&dev, 2, "       500      ", 16, false);
                        puls=500;
                }

                if (pot >= 750 && pot < 1000){
                        ssd1306_display_text(&dev, 2, "       750      ", 16, false);
                        puls=750;
                }

                if (pot >= 1000){
                        ssd1306_display_text(&dev, 2, "       1000     ", 16, false);
                        puls=1000;
                }

        if(pot > 250){

        timer.freq_hz=puls;
        ledc_timer_config(&timer);

                ledc_set_duty(pwm.speed_mode, pwm.channel, 512);
                ledc_update_duty(pwm.speed_mode, pwm.channel);
                ssd1306_display_text(&dev, 0, "     Girando", 12, false);   
        }
                else{
                ledc_set_duty(pwm.speed_mode, pwm.channel, 0);
                ledc_update_duty(pwm.speed_mode, pwm.channel);
                ssd1306_clear_line(&dev, 0, false);
                ssd1306_contrast(&dev, 0xff);
                }
             

}

}
   