/*
Author: Dejan Levec <dejan@dejanlevec.com>
Year: 2012
*/
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <process.h>
#include "FMOD/fmod.h"
#include <math.h> //for mod10

HANDLE comPort;
#define SPECTRUMSIZE 8192
#define OUTPUTRATE 48000

HANDLE stopAppMutex;
BOOL _stopApp;

HANDLE spectrumMutex;
float analyzedSpectrum[8],previousAnalyzedSpectrum[8];

unsigned char buf[3*25];

void set_color(int led, char r, char g, char b) {
	buf[led*3+0]=r;
	buf[led*3+1]=g;
	buf[led*3+2]=b;
}

/*
Function maps bars to actual LED lights
Bar [0-4]
Size [1-5]

LEDs:

4   5   14   15  24
3   6   13   16  23
2   7   12   17  22
1   8   11   18  21
0   9   10   19  20
*/
void barsToLEDMapping(int bar, int size) {
	int i;

	int r,r1,r2;
	
	r1= rand()%255;
	r2= rand()%255;
	r= rand()%255;

	switch(bar) {
	case 0:
		for(i = 0;i<size && i<5;i++) {
			set_color(i, r, r1, r2);
		}
		break;

	case 1:
		for(i = 9;i>9-size && i>=5&&i<=9;i--) {
			set_color(i, r, r1, r2);
		}
		break;
	case 2:
		for(i = 10;i<10+size && i<=14&&i>=10;i++) {
			set_color(i, r, r1, r2);
		}
		break;
	case 3:
		for(i = 19;i>19-size && i>=15&&i<=19;i--) {
			set_color(i, r, r1, r2);
		}
		break;
	case 4:
		for(i = 20;i<20+size && i>=20&&i<=24;i++) {
			set_color(i, r, r1, r2);
		}
		break;
	}
}

//FMOD Thread
void getSpectrum(void *fm) {
	FMOD_SYSTEM *fmod = (FMOD_SYSTEM*)fm;
	FMOD_CHANNEL *channel = 0;
	FMOD_SOUND *sound = 0;
	FMOD_CREATESOUNDEXINFO exinfo;
	int numdrivers, selected=-1;//for stereo mix
	int count;//stereo mix
	char name[256]; //stereo mix
	float t[SPECTRUMSIZE/8];
	int j = 0;
	int i = 0;
	float max=0;

	FMOD_RESULT result;
	static float spectrum[SPECTRUMSIZE];

	result = FMOD_System_SetOutput(fmod, FMOD_OUTPUTTYPE_DSOUND); //output=directsound

	//get stereo mix
	result = FMOD_System_GetRecordNumDrivers(fmod, &numdrivers);

	for (count=0; count < numdrivers; count++)
    {
        result = FMOD_System_GetRecordDriverInfo(fmod, count, name, 256, 0);
        
		if(strncmp(name, "Stereo Mix", strlen("Stereo Mix"))==0) {
			selected=count;
		}
    }

	if(selected == -1) {
		printf("Couldn't find Stereo Mix!\n");
		exit(-1);
	}

	//create a sound
	result = FMOD_System_SetSoftwareFormat(fmod, OUTPUTRATE, FMOD_SOUND_FORMAT_PCM16, 1, 0, FMOD_DSP_RESAMPLER_LINEAR);
    
	result = FMOD_System_Init(fmod, 100, FMOD_INIT_NORMAL, 0);

	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = 1;
    exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
    exinfo.defaultfrequency = OUTPUTRATE;
    exinfo.length           = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 5;
    
    result = FMOD_System_CreateSound(fmod, 0, FMOD_2D | FMOD_SOFTWARE | FMOD_LOOP_NORMAL | FMOD_OPENUSER, &exinfo, &sound);

	result = FMOD_System_RecordStart(fmod, selected, sound, TRUE);
	Sleep(200);
	result = FMOD_System_PlaySound(fmod, FMOD_CHANNEL_REUSE, sound, FALSE, &channel);
	result = FMOD_Channel_SetVolume(channel, 0);

	do {
		result = FMOD_Channel_GetSpectrum(channel, spectrum, SPECTRUMSIZE, 0, FMOD_DSP_FFT_WINDOW_TRIANGLE);

		
		for(i = 0;i<8000;i++) {
			t[j]+=(10.0f * (float)log10(spectrum[i]) * 2.0f);

			if(i<=1600) j=0;
			if(i>1600)j=1;
			if(i>3200)j=2;
			if(i>4800)j=3;
			if(i>6400)j=4;
			
		}

		max=0;

		WaitForSingleObject(spectrumMutex, 1);
		for(i =0;i<5;i++) {
			t[i]=t[i]/1600+150;
			

			analyzedSpectrum[i] = t[i];

			if(t[i] >max) {max=t[i];}
		}
		ReleaseMutex(spectrumMutex);
		

		FMOD_System_Update(fmod);
		Sleep(10);

		WaitForSingleObject(stopAppMutex, 1);
		if(_stopApp==TRUE) {break;}
		ReleaseMutex(stopAppMutex);
	}while(1);

	_endthread();
}


//COM communication
int serial_start(wchar_t *port) {
	DCB settings;
	int r;

	comPort = CreateFile( // http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
		port,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(comPort == INVALID_HANDLE_VALUE) {
		return -1;
	}

	r = GetCommState(
		comPort,
		&settings
		);

	if(r == 0) {
		return -1;
	}


	/*
	AVR: UCSR0C = (3 << UCSZ00); = no parity, 8 bits
	*/
	settings.BaudRate=128000;
	settings.ByteSize=8;
	settings.Parity=NOPARITY;
	settings.StopBits=ONESTOPBIT;

	r = SetCommState(
		comPort,
		&settings
		);

	if(r == 0) {
		return -1;
	}

	return 0;
}

void serial_write(unsigned char *buf, unsigned int length) {
	DWORD written = 0;
	WriteFile( // http://msdn.microsoft.com/en-us/library/windows/desktop/aa365747(v=vs.85).aspx
		comPort,
		buf,
		length,
		&written,
		NULL
	);
}

//Main program
int main() {
	
	int i,n=0,j=0;
	float diff;
	FMOD_SYSTEM *fmod = 0;
	FMOD_RESULT result;


	//establish COM communication
	if(serial_start(TEXT("COM7")) != 0) {
		printf("Error while establishing COM connection!\n");
		system("PAUSE");
		exit(1);
	}


	Sleep(1000);//sleep so that uC can clear LEDs' status

	//FMOD
	

	result = FMOD_System_Create(&fmod);

	if(result != FMOD_OK) {
		printf("FMOD Error");
		exit(1);
	}

	//Mutex
	stopAppMutex = CreateMutex(NULL,NULL,NULL);
	_stopApp = FALSE;

	spectrumMutex = CreateMutex(NULL,NULL,NULL);
	for(i = 0;i<8;i++) {
		analyzedSpectrum[i] = 0;
	}

	//FMOD listening thread
	_beginthread(getSpectrum, 0, fmod);

	//Network thread
	//_beginthread(notifyLightshow, 0, NULL);

	//control LEDs
	n=0;

	
	while(1) {
		//clear
		for(i = 0;i<25;i++){
			buf[0+i*3]=0x00;
			buf[1+i*3]=0x00;
			buf[2+i*3]=0x00;
		}

		WaitForSingleObject(spectrumMutex, 1);
		for( i =0;i<5;i++) {
			diff = abs((analyzedSpectrum[i]/10-previousAnalyzedSpectrum[i]/10)*10);

			switch(i) {
				case 0:
					barsToLEDMapping(0,diff);

					break;
				case 1:
					barsToLEDMapping(1,diff);
					break;
				case 2:
					barsToLEDMapping(2,diff);
					break;
				case 3:
					barsToLEDMapping(3,diff);
					break;
				case 4:
					barsToLEDMapping(4,diff);
					break;
			}

			previousAnalyzedSpectrum[i]=analyzedSpectrum[i];
		}
		ReleaseMutex(spectrumMutex);

		serial_write(buf, 3*25);
	}

	//cleanup
	CloseHandle(comPort);

	system("PAUSE");
	return 0;
}