/* FPGA Text LCD Test Application
File : fpga_test_text_lcd.c*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>

#define MAX_DIGIT 4  //fnd
#define MAX_BUTTON 9 //push button
#define LED_DEVICE "/dev/fpga_led"
#define FND_DEVICE "/dev/fpga_fnd"
#define PUSH_SWITCH_DEVICE "/dev/fpga_push_switch"
#define BUZZER_DEVICE "/dev/fpga_buzzer"
#define PASSWORD 1258

unsigned char quit = 0;

void user_signal1(int);
int openDevice(int *, int *, int *, int *);
void closeDevice(int *, int *, int *, int *);
int checkPushSwitch(int *, int, char *);

int main(void)
{
    int devPushSwitch, devLed, devFnd, devBuzzer; //devices
    unsigned char push_switch_data[4];            //fnd buffer
    unsigned char led_data, buzzer_data;
    int buff_size;                          //push switch buffer
    unsigned char push_sw_buff[MAX_BUTTON]; //push switch
    int num;                                //누른 스위치 숫자
    int curr_numberof_switch = 0;           //현재 누른 스위치 개수

    buff_size = sizeof(push_sw_buff);
    memset(push_switch_data, 0, sizeof(push_switch_data)); //fnd buffer

    openDevice(&devPushSwitch, &devLed, &devFnd, &devBuzzer); //device open

    (void)signal(SIGINT, user_signal1); //Ctrl+C 종료

    printf("Press <ctrl+c> to quit. \n");
    while (!quit)
    {
        usleep(400000);
        //push_switch가 눌렸는지 확인후 눌렸다면 1이상 수 반환
        if (num = checkPushSwitch(devPushSwitch, buff_size, &push_sw_buff) > 0)
        {
            if (curr_numberof_switch < 4)
            {
                push_switch_data[curr_numberof_switch] = num - '0'; //fnd data숫자 바꾸고 device에 write
                write(devFnd, &push_switch_data, 4);
            }
        }
        //4자리 다 눌렀다면
        if (curr_numberof_switch > 3)
        {
            //Password가 같다면
            if (PASSWORD == atoi(push_switch_data))
            {
                //Led
                for (int i = 0; i < 8; i++)
                {
                    led_data = (int)pow((double)2, (double)i);
                    write(devLed, &led_data, 1);
                }
                led_data = 0;
                write(devLed, &led_data, 1);
                led_data = 255;
                write(devLed, &led_data, 1);
            }
            else
            {
                memset(push_switch_data, 0, sizeof(push_switch_data)); //4이상이면 초기화하고 다시 시작
                write(devFnd, &push_switch_data, 4);
                buzzer_data = 1;
                write(devBuzzer, &buzzer_data, 1); //Buzzer
            }
        }
    }
    closeDevice(&devPushSwitch, &devLed, &devFnd, &devBuzzer);
}

void user_signal1(int sig)
{
    quit = 1;
}

int openDevice(int *devPushSwitch, int *devLed, int *devFnd, int *devBuzzer)
{
    *devPushSwitch = open(PUSH_SWITCH_DEVICE, O_RDWR);
    *devLed = open(LED_DEVICE, O_RDWR);
    *devFnd = open(FND_DEVICE, O_RDWR);
    *devBuzzer = open(BUZZER_DEVICE, O_RDWR);

    if (*devPushSwitch < 0)
    {
        printf("Push Switch Device Open Error\n");
        close(devPushSwitch);
        return 1
    }
    if (*devLed < 0)
    {
        printf("LED Device Open Error\n");
        close(*devLed);
        return 1;
    }
    if (*devFnd < 0)
    {
        printf("FND Device Open Error\n");
        close(*devFnd);
        return 1;
    }
    if (*devBuzzer < 0)
    {
        printf("Buzzer Device Open Error\n");
        close(*devBuzzer);
        return 1;
    }

    return 0;
}

void closeDevice(int *devPushSwitch, int *devLed, int *devFnd, int *devBuzzer)
{
    close(devPushSwitch);
    close(devLed;
    close(devFnd);
    close(devBuzzer);
}

int checkPushSwitch(int *devPushSwitch, int buff_size, char *push_sw_buff)
{
    read(*devPushSwitch, push_sw_buff, buff_size);

    for (int i = 0; i < MAX_BUTTON; i++)
    {
        if (push_sw_buff[i] > 0)
            return i + 1;
    }
    return 0;
}