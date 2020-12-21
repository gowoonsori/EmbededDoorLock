#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>      //pow함수
#include <signal.h>    //ctrl+c 로 종료
#include <time.h>      //로그를 남길때 시간 사용
#include <sys/timeb.h> //msec
#include <pthread.h>   //Siren thread

#define MAX_DIGIT 4 //비밀번호 자리수
#define LED_DEVICE "/dev/fpga_led"
#define FND_DEVICE "/dev/fpga_fnd"
#define PUSH_SWITCH_DEVICE "/dev/fpga_push_switch"
#define BUZZER_DEVICE "/dev/fpga_buzzer"
#define STEP_MOTOR_DEVICE "/dev/fpga_step_motor"

unsigned char quit = 0; //ctrl+c로 종료하기위한 flag변수

void user_signal1(int);                       //ctrl+c
int openDevice();                             //Device 5개 open과 초기화(모두 off)
void closeDevice(int *, int *, int *, int *); //Device 5개 close
int checkPushSwitch(int *, unsigned char *);  //push_switch눌렀을때 몇번째가 눌렸는지 검사

void *sirenThread(void *arg); //비밀번호 틀렸다면 모터와 부저 작동하고 언제든 원할때 끌 수 있게
void buffer_flush();          //buffer flush함수

int devPushSwitch, devLed, devFnd, devBuzzer, devMotor; //devices
int PASSWORD;                                           //초기 비밀번호

int main(int argc, char **argv)
{
    /*초기 비밀번호 설정*/
    if (argc != 2)
    {
        printf("please input the parameter! \n");
        printf("ex)./doorlock 1234\n");
        return -1;
    }

    int str_size = (strlen(argv[1]));
    if (str_size > MAX_DIGIT)
    {
        printf("Warning! 4 Digit number only!\n");
        str_size = MAX_DIGIT;
    }

    for (int i = 0; i < str_size; i++)
    {
        if ((argv[1][i] < 0x30) || (argv[1][i]) > 0x39)
        {
            printf("Error! Invalid Value!\n");
            return -1;
        }
    }
    PASSWORD = atoi(argv[1]);

    unsigned char push_switch_data[4]; //fnd buffer
    unsigned char led_data = 0;        //led data buffer
    unsigned char push_sw_buff[1];     //push switch
    int num;                           //누른 스위치 숫자
    int curr_numberof_switch = 0;      //현재 누른 스위치 개수
    pthread_t siren_thd;

    memset(push_switch_data, 0, sizeof(push_switch_data)); //fnd buffer 0초기화

    openDevice(); //device open과 동시에 기존 장치들 off상태

    (void)signal(SIGINT, user_signal1); //Ctrl+C 종료시그널 등록
    printf("Press <ctrl+c> to quit. \n");
    while (!quit)
    {
        usleep(400000); //0.4초
        //push_switch가 눌렸는지 확인후 눌렸다면 1이상 수 반환
        if ((num = checkPushSwitch(&devPushSwitch, push_sw_buff)) > 0)
        {
            if (curr_numberof_switch < 4) //만일 5자리 이상 쓸 경우 대비한 조건
            {
                push_switch_data[curr_numberof_switch] = num + '0'; //fnd data숫자 바꾸고 device에 write
                write(devFnd, &push_switch_data, 4);
                curr_numberof_switch++;
            }
        }

        //4자리 다 눌렀다면
        if (curr_numberof_switch > 3)
        {
            int push_password = atoi(push_switch_data);

            curr_numberof_switch = 0; //switch_buffer 초기화
            memset(push_switch_data, 0, sizeof(push_switch_data));
            write(devFnd, &push_switch_data, 4);

            //Password가 같다면
            if (PASSWORD == push_password)
            {
                struct timeb itb;
                ftime(&itb);
                struct tm *tm = localtime(&itb.time);
                printf("\n!! %02d:%02d:%02d:%03d 잠금장치가 열렸습니다. !!\n", tm->tm_hour, tm->tm_min, tm->tm_sec, itb.millitm);

                //Led 이쁘게 출력
                for (int i = 0; i < 8; i++)
                {
                    led_data = (int)pow((double)2, (double)i);
                    write(devLed, &led_data, 1);
                    usleep(100000);
                }
                usleep(200000);
                led_data = 255;
                write(devLed, &led_data, 1);
                usleep(200000);
                led_data = 0;
                write(devLed, &led_data, 1);
                usleep(200000);
                led_data = 255;
                write(devLed, &led_data, 1);
                usleep(1000000);
                led_data = 0;
                write(devLed, &led_data, 1);
            }
            else
            {
                struct timeb itb;
                ftime(&itb);
                struct tm *tm = localtime(&itb.time);
                printf("\n!! %02d:%02d:%02d:%03d 비밀번호를 틀렸습니다. !!\n", tm->tm_hour, tm->tm_min, tm->tm_sec, itb.millitm);

                pthread_create(&siren_thd, NULL, sirenThread, NULL);
                pthread_detach(siren_thd);
            }
        }
    }
    closeDevice(&devPushSwitch, &devLed, &devFnd, &devBuzzer);
}

void *sirenThread(void *arg)
{
    int isDigit, menuItem;
    unsigned char buzzer_data = 1;
    unsigned char buzzer_on;
    unsigned char motor_state[3];

    motor_state[0] = (unsigned char)1;   //motor on
    motor_state[1] = (unsigned char)1;   //오른쪽으로 회전
    motor_state[2] = (unsigned char)255; //motor 최고 속도

    write(devBuzzer, &buzzer_data, 1); //Buzzer start
    write(devMotor, motor_state, 3);   //motor start

    /*Buzzer가 소리가 안나오는 관계로 read함수를 통해 Buzzer 작동 여부 판단*/
    read(devBuzzer, &buzzer_on, 1);
    if ((int)buzzer_on > 0)
        printf("\n !! buzzer ON !!\n");

    /*사이렌 종료 입력시까지 사이렌 작동*/
    while (1)
    {
        fprintf(stdout, "\n사이렌 종료 : 1번 클릭\n");
        isDigit = scanf("%d", &menuItem); //숫자면 isDigit =1
        buffer_flush();
        if (menuItem == 1 && isDigit == 1)
        {
            buzzer_data = 0;
            motor_state[0] = (unsigned)0;
            write(devBuzzer, &buzzer_data, 1);
            write(devMotor, motor_state, 3);
            break;
        }
        fprintf(stdout, "잘못된 입력입니다.\n");
    }
}

void user_signal1(int sig)
{
    quit = 1;
}

int openDevice()
{
    devPushSwitch = open(PUSH_SWITCH_DEVICE, O_RDWR);
    devLed = open(LED_DEVICE, O_RDWR);
    devFnd = open(FND_DEVICE, O_RDWR);
    devBuzzer = open(BUZZER_DEVICE, O_RDWR);
    devMotor = open(STEP_MOTOR_DEVICE, O_RDWR);

    if (devPushSwitch < 0)
    {
        printf("Push Switch Device Open Error\n");
        close(devPushSwitch);
        exit(1);
    }
    if (devLed < 0)
    {
        printf("LED Device Open Error\n");
        close(devLed);
        exit(1);
    }
    if (devFnd < 0)
    {
        printf("FND Device Open Error\n");
        close(devFnd);
        exit(1);
    }
    if (devBuzzer < 0)
    {
        printf("Buzzer Device Open Error\n");
        close(devBuzzer);
        exit(1);
    }
    if (devMotor < 0)
    {
        printf("Motor Device Open Error\n");
        close(devMotor);
        exit(1);
    }
    unsigned char push_switch_data[4]; //fnd buffer
    unsigned char led_data = 0;        //led data buffer
    unsigned char buzzer_data = 0;
    unsigned char motor_state[3];
    memset(push_switch_data, 0, sizeof(push_switch_data)); //fnd buffer 0초기화

    motor_state[0] = (unsigned char)0;   //motor on
    motor_state[1] = (unsigned char)1;   //오른쪽으로 회전
    motor_state[2] = (unsigned char)255; //motor 최고 속도

    write(devFnd, &push_switch_data, 4);
    write(devLed, &led_data, 1);
    write(devBuzzer, &buzzer_data, 1);
    write(devMotor, &motor_state, 3);

    return 0;
}

void closeDevice(int *devPushSwitch, int *devLed, int *devFnd, int *devBuzzer)
{
    close(*devPushSwitch);
    close(*devLed);
    close(*devFnd);
    close(*devBuzzer);
}

int checkPushSwitch(int *devPushSwitch, unsigned char *push_sw_buff)
{
    //총 9개의 push switch의 정보중 누른 스위치 정보 read
    read(*devPushSwitch, push_sw_buff, 9);
    return (unsigned int)push_sw_buff[0];
}

void buffer_flush()
{
    while (getchar() != '\n')
        ;
}