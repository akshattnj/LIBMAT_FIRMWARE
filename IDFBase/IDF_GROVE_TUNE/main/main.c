/*macro definition of Speaker pin
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define SPEAKER 23

void pinInit(void);
void sound(uint8_t note_index);

int BassTab[]={1911,1702,1516,1431,1275,1136,1012};//bass 1~7

void app_main(void)
{
    pinInit();
    while (1) {
        //sound bass 1~7
        for(int note_index=0;note_index<7;note_index++)
        {
            sound(note_index);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void pinInit()
{
    gpio_pad_select_gpio(SPEAKER);
    gpio_set_direction(SPEAKER, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKER, 0);
}

void sound(uint8_t note_index)
{
    for(int i=0;i<100;i++)
    {
        gpio_set_level(SPEAKER, 1);
        vTaskDelay(pdMS_TO_TICKS(BassTab[note_index]/2000)); // divide by 2000 to convert microseconds to milliseconds
        gpio_set_level(SPEAKER, 0);
        vTaskDelay(pdMS_TO_TICKS(BassTab[note_index]/2000));
    }
}
*/


#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

//macro definition of Speaker pin
#define SPEAKER 23

// Note frequencies 
#define C4   262
#define D4   294
#define E4   330
#define F4   349
#define G4   392
#define Gs4  415
#define A4   440
#define Bb4  466
#define B4   494
#define C5   523
#define Cs5  554
#define D5   587
#define Ds5  622
#define E5   659
#define F5   698
#define Fs5  740
#define G5   784
#define Gs5  831
#define A5   880

#define G3  196
#define E3  165
#define A3  220
#define B3  247
#define As3 185

// 
#define Q   400     // quarter note
#define H   2*Q     // half note
#define W   4*Q     // whole note
#define E   Q/2     // eighth note


typedef struct {
  int frequency;
  int duration;
} Note;


void pinInit(void);
void play(Note note);
void rest(int duration);

void app_main(void)
{
    pinInit();
    while (1){
    // Super Mario Bros theme song
    play((Note) {E4, Q});
    play((Note) {E4, Q});
    rest(Q);
    play((Note) {E4, Q});
    rest(Q);
    play((Note) {C4, Q});
    play((Note) {E4, Q});
    rest(Q);
    play((Note) {G4, H});
    rest(Q);
    play((Note) {G3, H});
    rest(Q);
    play((Note) {C4, H});
    rest(Q);
    play((Note) {G3, H});
    rest(Q);
    play((Note) {E3, Q});
    play((Note) {A3, Q});
    rest(Q);
    play((Note) {B3, H});
    rest(Q);
    play((Note) {As3, Q});
    play((Note) {A3, Q});
    rest(Q);
    play((Note) {G3, E});
    rest(E);
    play((Note) {E4, Q});
    play((Note) {G4, H});
    rest(Q);
    play((Note) {A4, Q});
    rest(Q);
    play((Note) {F4, Q});
    play((Note) {G4, E});
    rest(E);
    play((Note) {E4, Q});
    play((Note) {C4, Q});
    play((Note) {D4, Q});
    play((Note) {B3, Q});
    rest(Q);
    play((Note) {C4, H});
    rest(Q);
    play((Note) {G3, H});
    rest(Q);
    play((Note) {E3, Q});
    play((Note) {A3, Q});
    rest(Q);
    play((Note) {B3, H});
    rest(Q);
    play((Note) {As3, Q});
    play((Note) {A3, Q});
    rest(Q);
    play((Note) {G3, E});
    rest(E);
    play((Note) {E4, Q});
}
}

void pinInit(void)
{
    gpio_pad_select_gpio(SPEAKER);
    gpio_set_direction(SPEAKER, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKER, 0);
}

void play(Note note)
{
    if (note.frequency == 0) {
        rest(note.duration);
    } else {
        int half_period = 1000000 / (2 * note.frequency);
        int cycles = note.duration * note.frequency / 1000;
        for (int i = 0; i < cycles; i++) {
            gpio_set_level(SPEAKER, 1);
            ets_delay_us(half_period);
            gpio_set_level(SPEAKER, 0);
            ets_delay_us(half_period);
        }
    }
}

void rest(int duration)
{
    vTaskDelay(duration / portTICK_PERIOD_MS);
}