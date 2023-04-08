#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include "Randomizer.h"
#include <locale.h>
#define WATER_LEVEL 0
#define GAS_LEVEL 1
#define TEMPER_LEVEL 2
#define BIT_VALUES 3
#define SECONDS_15 75
#pragma comment(lib, "ws2_32.lib")
//g++ *.cpp -lws2_32 -o PLCSignalsEmulator.exe

Randomizer random;
SOCKET sConnection;
// Контейнер для отправки данных
union MSGToSend
{
	unsigned short plcSgnls[4];
	char msgToServer[sizeof(plcSgnls)];
};
// Контейнер для получения данных
union MSGToRecv
{
	char msgFromSrv[sizeof(short)];
	unsigned short plcSBitValues;
};

MSGToSend msgToSend;
MSGToRecv msgToRecv;
unsigned short plcSBitValues, plcSBitValuesOld;

bool working, fireDanger;
// Поток чтения сообщения от сервера
void msgFromServer()
{
	unsigned short bitArray, bitArrayOld;
	while(working)
	{
		try
		{
			if(recv(sConnection, msgToRecv.msgFromSrv, sizeof(short), NULL) > 0) 
			{
				plcSBitValues = msgToRecv.plcSBitValues;
				if(plcSBitValuesOld != plcSBitValues)
				{
					for(int i = 0; i < 16; i++)
					{
						bitArray = plcSBitValues;
						bitArray = bitArray << i;
						bitArray = bitArray >> 15;

						bitArrayOld = plcSBitValuesOld;
						bitArrayOld = bitArrayOld << i;
						bitArrayOld = bitArrayOld >> 15;

						if(bitArray != bitArrayOld)
						{
							msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | (bitArray << i);
							if(bitArray == 0) msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ (bitArray << i);
						}
					}
				}

				bitArray = plcSBitValues;
				bitArray = bitArray << 15;
				bitArray = bitArray >> 15;
				fireDanger = bitArray != 0;

				bitArray = plcSBitValues;
				bitArray = bitArray << 14;
				bitArray = bitArray >> 15;
				fireDanger = bitArray != 0;

				bitArray = plcSBitValues;
				bitArray = bitArray >> 15;
				working = bitArray == 0;

				plcSBitValuesOld = plcSBitValues;
			}
			else throw 1;
		}
		catch(...)
		{
			printf("%s\n", "Exception on reciving data");
		}
		Sleep(250);
	}
}
// Поток отправки сообщений на сервер
void msgToServer()
{
	while(working)
	{
		try
		{
			if (send(sConnection, msgToSend.msgToServer, sizeof(msgToSend.plcSgnls), NULL) == 0) throw 1;
		}
		catch(...)
		{
			printf("%s\n", "Exception on sending data");
		}
		Sleep(200);
	}
}
// Основной поток программы
int main(int argc, char **argv)
{
	// Инициализация
	setlocale(LC_ALL, "russian");
	printf("%d\n", random.result(2000));
	WSAData wsdata;
	WORD DLLVersion = MAKEWORD(2, 1);
	if(WSAStartup(DLLVersion, &wsdata) != 0)
	{
		printf("Loading library failed!\n");
		exit(1);
	}

	printf("Loading library success!\n");

	SOCKADDR_IN addrs;
	addrs.sin_addr.s_addr = inet_addr("127.0.0.1");
	addrs.sin_port = htons(1111);
	addrs.sin_family = AF_INET;
	int sizeofaddrs = sizeof(addrs);

    printf("Socket boot success\n");

    sConnection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    while(connect(sConnection, (SOCKADDR*)&addrs, sizeofaddrs) != 0)
    {
       printf("No server connection\n");
       Sleep(2000); 
    }
    printf("Server connection success\n");

	// Установка случайных величин для уровня воды, газа и температуры
	msgToSend.plcSgnls[WATER_LEVEL] = random.result(2000);
	msgToSend.plcSgnls[GAS_LEVEL] = 100;
	msgToSend.plcSgnls[TEMPER_LEVEL] = 15 + random.result(30);
	msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x10;
	msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x20;
	unsigned short oldBitValues, bitArray;
	int cycleCounter = 75;
	working = true;
	fireDanger = false;

	printf("Variables creation success\n");
	// Запуск потоков
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)msgFromServer, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)msgToServer, NULL, NULL, NULL);
	// Рабочий цикл
	while(working)
	{	
		// Эмуляция изменения температуры
		bitArray = msgToSend.plcSgnls[BIT_VALUES];
		bitArray = bitArray << 9;
		bitArray = bitArray >> 15;
		msgToSend.plcSgnls[TEMPER_LEVEL] = bitArray == 1 ? msgToSend.plcSgnls[TEMPER_LEVEL] : random.result(100) > 65 ? ++msgToSend.plcSgnls[TEMPER_LEVEL] : msgToSend.plcSgnls[TEMPER_LEVEL];
		// Эмуляция изменения уровня воды
		bitArray = msgToSend.plcSgnls[BIT_VALUES];
		bitArray = bitArray << 5;
		bitArray = bitArray >> 15;
		msgToSend.plcSgnls[WATER_LEVEL] = bitArray == 1 ? msgToSend.plcSgnls[WATER_LEVEL] : random.result(100) > 50 ? ++msgToSend.plcSgnls[WATER_LEVEL] : msgToSend.plcSgnls[WATER_LEVEL];

		if(fireDanger) goto FireAlarm;
		// Эмуляция срабатывания датчиков дыма
		msgToSend.plcSgnls[BIT_VALUES] = random.result(1000) > 998 ? msgToSend.plcSgnls[BIT_VALUES] | 0x1 : msgToSend.plcSgnls[BIT_VALUES];
		msgToSend.plcSgnls[BIT_VALUES] = random.result(1000) > 998 ? msgToSend.plcSgnls[BIT_VALUES] | 0x2 : msgToSend.plcSgnls[BIT_VALUES];

		bitArray = msgToSend.plcSgnls[BIT_VALUES];
		bitArray = bitArray << 15;
		bitArray = bitArray >> 15;
		if(bitArray == 1) goto FireAlarm;

		bitArray = msgToSend.plcSgnls[BIT_VALUES];
		bitArray = bitArray << 14;
		bitArray = bitArray >> 15;
		if(bitArray == 1) goto FireAlarm;

		fireDanger = false;

		goto standartWorking;

		FireAlarm:

		bitArray = msgToSend.plcSgnls[BIT_VALUES];
		bitArray = bitArray << 15;
		bitArray = bitArray >> 15;
		fireDanger = bitArray == 1;

		bitArray = msgToSend.plcSgnls[BIT_VALUES];
		bitArray = bitArray << 14;
		bitArray = bitArray >> 15;
		fireDanger = bitArray == 1;

		printf("ALARM FIRE!\n");
		// Реакция на пожарную тревогу
		if(cycleCounter > 74)
		{		
			Sleep(200);
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x4;
			Sleep(200);
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x8;
			Sleep(200);
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x10;
		    msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ 0x10;
			Sleep(200);
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x20;
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ 0x20;
			Sleep(200);
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x40;
		    msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ 0x40;
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x80;	
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ 0x80;
			Sleep(200);
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x400;
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ 0x400;
			Sleep(200);
		}

		if(cycleCounter > 0)
		{
			printf("Gas will be started in %d seconds\n", cycleCounter/5);
		}
		else
		{
			if(cycleCounter == 0) printf("Gas started\n");
			if(msgToSend.plcSgnls[GAS_LEVEL] > 0) msgToSend.plcSgnls[GAS_LEVEL]--;
			if((msgToSend.plcSgnls[GAS_LEVEL] > 0) && ((msgToSend.plcSgnls[TEMPER_LEVEL]-15) > 15)) msgToSend.plcSgnls[TEMPER_LEVEL] = msgToSend.plcSgnls[TEMPER_LEVEL]-15;
		}

		if(cycleCounter > -1) --cycleCounter;

		standartWorking:
		// Стандартная режим работы
		printf("Byte massive in plc/from server = %d/%d\n", msgToSend.plcSgnls[BIT_VALUES], plcSBitValues);

		system("cls");

		if(fireDanger) continue;

		cycleCounter = 75;

		msgToSend.plcSgnls[GAS_LEVEL] = 100;

		if(msgToSend.plcSgnls[TEMPER_LEVEL] > 40) 
		{
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x40;
			msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x80;
		}
		if(msgToSend.plcSgnls[WATER_LEVEL] > 1500) msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] | 0x400;

		bitArray = msgToSend.plcSgnls[BIT_VALUES];
		bitArray = bitArray << 9;
		bitArray = bitArray >> 15;
		if(bitArray == 1)
		{
			if(msgToSend.plcSgnls[TEMPER_LEVEL] < 20)
			{
				msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ 0x40;
				msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ 0x80;
			}
			msgToSend.plcSgnls[TEMPER_LEVEL]--;
		}

		bitArray = msgToSend.plcSgnls[BIT_VALUES];
		bitArray = bitArray << 5;
		bitArray = bitArray >> 15;
		if(bitArray == 1)
		{
			if(msgToSend.plcSgnls[WATER_LEVEL] < 100)
			{
				msgToSend.plcSgnls[BIT_VALUES] = msgToSend.plcSgnls[BIT_VALUES] ^ 0x400;
			}
			if (msgToSend.plcSgnls[WATER_LEVEL]-10 > 0) msgToSend.plcSgnls[WATER_LEVEL] = msgToSend.plcSgnls[WATER_LEVEL]-10;
		}
	}
	
	WSACleanup();
	system("pause");
	return 0;
}