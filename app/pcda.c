#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <fcntl.h> //文件控制
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 

#define  PowerOn     0x01
#define  PowerOff    0x02
#define  XfrAPDU     0x03

#define SLOT_ICC     0
#define SLOT_SAM1    1
#define SLOT_SAM2    2
#define SLOT_SAM3    3

#define PCD_CMD(n,m) ((n << 4) | m)

#define Debug

#ifdef Debug
#define PrtMsg(arg...) printf(arg)
#else
#define PrtMsg(arg...)
#endif

unsigned char RecBuf[271];
unsigned char CmdBuf[271];
int fd;

typedef struct
{
    unsigned char *p_iBuf;
    unsigned char *p_oBuf;
    unsigned int  iDataLen;
    unsigned int  oDataLen;

}IFD_PARAM;

IFD_PARAM UsrParam;



unsigned char StrToHex(char* src, int len)
{
   int i=0;
   unsigned char a=0;

   if(src == NULL)   return 0;
   else
   {
      while(len--)
      {
//        PrtMsg("src + %d = 0x%X\n", i, *(src+i));
        if(*(src + i) >= '0' && *(src + i) <= '9')
            a = a*16 + (src[i]-'0');
        else if(*(src + i) >= 'a' && *(src + i) <= 'f')
            a = a*16 + (src[i]-'a')+10;
        else if(*(src + i) >= 'A' && *(src + i) <= 'F')
            a = a*16 + (src[i]-'A')+10;
        else if(*src + i == 'x' || *src + i == 'X');
            
        ++i;
      }
   return a;
   }
}

unsigned char ArrayCompare(unsigned char* arr1, unsigned char* arr2, unsigned int len)
{
    while(len)
    {
        if(*arr1 != *arr2)   return(0);
        else 
        {
            arr1++;
            arr2++;
            len--;
        }
    }
    return(1);
}

unsigned char CardPowerOn(unsigned char slot)
{
    unsigned char retry;
    long retval;
    int i;

    retry = 0;
    do
    {
        ioctl(fd, PCD_CMD(PowerOff, slot), &UsrParam);
        usleep(50000);
        UsrParam.oDataLen = 32;
        if((retval = ioctl(fd, PCD_CMD(PowerOn, slot), &UsrParam)) >= 0)
        {
            break;
        }
        retry++;
    }while(retry < 3);

    if(retry >= 3)
    {
        printf("\nOperation fail with errorcode = %lX\n", retval);
        return(0);
    }
    else
    {
        printf("\n\nATR Len: %d\n", UsrParam.oDataLen);
        printf("slot%d ATR:", slot);
        for(i = 0; i < UsrParam.oDataLen; i++ )
        {
            printf(" 0x%02X", RecBuf[i]);
        }
        printf("\n\n");
        return(1);
    }
}

void CardOPeration(int choice1, unsigned char slot)
{
    long retval;
    FILE * txtfd;
    char fbuf[1024];
    char *pBuf;
    int i;
    int TempLen;
    int choice2;

    UsrParam.p_oBuf = RecBuf;
    UsrParam.p_iBuf = CmdBuf;


    switch(choice1)
    {
        case '1':
        {
            CardPowerOn(slot);
            break;            
        }
        case '2':
        {
            ioctl(fd, PCD_CMD(PowerOff, slot), &UsrParam);
            break;
        }
        case '3':
        {
            do
            {
                printf("Exchange Apdu with slot%d:\n",slot);
                printf("==============================================\n\n");
                printf("1: Mifare Desfire Test\n");
                printf("2: Felica Test 1\n");
                printf("3: Felica Test 2\n");
                printf("4: Felica Test 3\n");
                printf("5: Felica Test 4\n");
                printf("6: Mifare 1k Test\n");
                printf("7: TypeB Test2\n");
                printf("\n");
                printf("0: Exit the test program\n\n");
                printf("==============================================\n\n");

                choice2 = getc(stdin);
                if(choice2 == 10)
                {
                   choice2 = getc(stdin);
                }

                if(choice2 == '1')
                {
                    txtfd = fopen("desfire.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: desfire.txt\n");
                    }
                }
                else if(choice2 == '2')
                {
                    txtfd = fopen("felicaTest1.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: felicaTest1.txt\n");
                    }
                }
                else if(choice2 == '3')
                {
                    txtfd = fopen("felicaTest2.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: felicaTest2.txt\n");
                    }
                }
                else if(choice2 == '4')
                {
                    txtfd = fopen("felicaTest3.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: felicaTest3.txt\n");
                    }
                }
                else if(choice2 == '5')
                {
                    txtfd = fopen("felicaTest4.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: felicaTest4.txt\n");
                    }
                }
                else if(choice2 == '6')
                {
                    txtfd = fopen("mifare1k.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: mifare1k.txt\n");
                    }
                }               
                else if(choice2 == '7')
                {
                    txtfd = fopen("typeBTest2.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: typeBTest2.txt\n");
                    }
                }
                
                if(txtfd != NULL)
                {
                    while( fgets(fbuf, 1024, txtfd) != NULL)
                    {
                        if(*fbuf == ';')
                        {
                            printf("%s",fbuf);
                        }
                        else if(*fbuf == '\n');
                        else if(strstr(fbuf, ".RESET") != NULL)
                        {
                            CardPowerOn(slot);
                        }
                        else if(strstr(fbuf, "Command:") != NULL)
                        {
                            pBuf = fbuf + 9;
                            i = 0;
                            while(*pBuf != '\n' && *pBuf != ' ')
                            {
                                CmdBuf[i] = StrToHex(pBuf, 2);
                                pBuf += 3;
                                i++;
                            }
         
                            UsrParam.p_iBuf = CmdBuf;
                            UsrParam.iDataLen = i;
                            UsrParam.oDataLen = 271;

                            PrtMsg("\n\nCMD Len: %d\n", UsrParam.iDataLen);
                            PrtMsg("slot%d CMD:", slot);
                            for(i = 0; i < UsrParam.iDataLen; i++ )
                            {
                                PrtMsg("0x%02X ", UsrParam.p_iBuf[i]);
                            }
                            PrtMsg("\n\n");

                            if((retval = ioctl(fd, PCD_CMD(XfrAPDU, slot), &UsrParam)) < 0)
                            {
                                PrtMsg("\nOperation fail with errorcode = %lX\n", retval);
                                break;
                            }
                            else
                            {

                                PrtMsg("Response Length: %d\n", UsrParam.oDataLen);
                                PrtMsg("Response Data:");
                                for(i = 0; i < UsrParam.oDataLen; i++ )
                                {
                                    PrtMsg("0x%02X ", RecBuf[i]);
                                }
                                PrtMsg("\n\n");

                                fgets(fbuf, 1024, txtfd);
                                if(strstr(fbuf, "Response:") != NULL)
                                { 
                                    pBuf = fbuf + 10;
                                    i = 0;
                                    while(*pBuf != '\n' && *pBuf != ' ')
                                    {
                                        if(*pBuf == 'x' || *pBuf == 'X');
                                        else
                                        {
                                            if(RecBuf[i] != StrToHex(pBuf, 2))
                                            { 
                                                goto exchangefail;
                                            }
                                        }
                                        pBuf += 3;
                                        i++;
                                    }
                                    TempLen = i;

                                    if(TempLen == UsrParam.oDataLen)
                                    {
                                        PrtMsg("Success to Exchange with the slot%d!\n", slot);
                                        PrtMsg("\n\n");
                                    }
                                    else
                                    {
exchangefail:
                                        PrtMsg("Fail to Exchange with the slot%d!\n", slot);
                                        PrtMsg("The expected data:\n");
                                        PrtMsg("%s",fbuf);
                                        PrtMsg("\n\n");
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            
            }while(choice2 != '0');
  }
        break;

        default:
            printf("Invalid selection: %d\n",choice1);
    }
}

int main()
{
    int choice;
    unsigned char slot;



    fd=open("/dev/pcd",O_RDWR);
    if(-1 == fd)
    {
        perror("error open\n");
        return(-1);
    }

    do
    {
        printf("pcd test program:\n");
        printf("==============================================\n\n");
        printf("1: Power on PICC\n");
        printf("2: Power off PICC\n");
        printf("3: Exchange Apdu with PICC\n");
        printf("\n");
        printf("0: Exit the test program\n\n");
        printf("==============================================\n\n");

        choice = getc(stdin);
        if(choice == 10)   choice = getc(stdin);
        if((choice >= '1') && (choice <= '4'))
        {
            slot = 0;
            CardOPeration(choice, slot);
        }
    }while(choice != '0');
 
    close(fd);;
    return(0);
}
