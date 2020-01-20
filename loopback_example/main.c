// https://github.com/simonemarra/RS232_C_Loopback_Windows.git
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include "rs232.h"

// Buffers
int cmdPos = 0;
char cmdBuffer[1024];
char rxBuffer[1024];

int cport_nr=3; //  /dev/ttyS* (COM* on windows)
int bdrate=115200;  // 115200 baud
char mode[]={'8','N','1',0};

void Delay_ms(int ms);
int GSM_ComPortOpen();
void GSM_SendCommand(char* cmd);
int GSM_ReadResponse(int timeoutSeconds);

int main()
{
    printf("RS232_C_Loopback_Windows\r\n");
    if(GSM_ComPortOpen() < 0)
    {
        Delay_ms(2500);
        return -1;
    }

    while(1)
    {
        // Check console input...
        if (_kbhit())
        {
            cmdBuffer[cmdPos++] = (char)_getch();
            cmdBuffer[cmdPos] = '\0';
            if(cmdPos >= 1023)
            {
                cmdPos = 0;
            }
            else
            {
                //printf("%c", cmdBuffer[cmdPos-1]);
                if((cmdBuffer[cmdPos-1] == '\n') || (cmdBuffer[cmdPos-1] == '\r'))
                {
                    // is newLine, we should execute command...
                    printf("\r\n[GSM] >> %s\r\n", cmdBuffer);
                    GSM_SendCommand(cmdBuffer);
                    cmdPos = 0;
                    memset(cmdBuffer, '\0', 1024);
                }
            }
        }

        int n = GSM_ReadResponse(1);
        if(n > 0)
        {
            printf("Received %d bytes...", n);
            printf(">> %s\r\n",rxBuffer);
        }
    }
    RS232_CloseComport(cport_nr);


    return 0;
}


void Delay_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms*1000);  /* sleep for ms milliSecond */
#endif
}

int GSM_ComPortOpen()
{
    if(RS232_OpenComport(cport_nr, bdrate, mode, 0))
    {
        printf("Can not open comport %d\n", cport_nr);
        return(-1);
    }
    return(0);
}

void GSM_SendCommand(char* cmd)
{
    RS232_cputs(cport_nr, cmd);
}

int GSM_ReadResponse(int timeoutSeconds)
{
    int i, bytesRx = -1;
    time_t start_t, end_t;
    double diff_t;
    time(&start_t);
    do
    {
        int n = RS232_PollComport(cport_nr, rxBuffer, 1023);
        if(n > 0)
        {
            bytesRx += n;
            rxBuffer[n] = 0;   /* always put a "null" at the end of a string! */
            for(i=0; i < n; i++)
            {
                if(rxBuffer[i] != '\n' && rxBuffer[i] != '\r' && rxBuffer[i] < 32)  /* replace unreadable control-codes by dots */
                {
                    rxBuffer[i] = '.';
                }
            }
        }
        time(&end_t);
        diff_t = difftime(end_t, start_t);
    } while(diff_t < timeoutSeconds);
    return bytesRx;
}
