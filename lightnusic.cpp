//////////////////////////////////////////////////////////////////////////////////////////
// Developer : Dimas Toha Pramawitra (Lonehack)
//////////////////////////////////////////////////////////////////////////////////////////
// Compile :
// 		g++ lightmusic.cpp -lasound -o lightmusic `pkg-config --cflags --libs opencv`
//////////////////////////////////////////////////////////////////////////////////////////

//#include "opencv2/core/core.hpp"
//#include "opencv2/contrib/contrib.hpp"
//#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include <alsa/pcm.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#define BUFFER_LEN 9010

using namespace std;
using namespace cv;

/** Global Variable **/
const char *device = "default";		//speaker hw number
//double Scale = 1.04;
int Thresh = 235;					//light sensitivity (0-255)
int lotone;			           		//freq of lowest tone, low 261, mid 523, hi 1046
int hitone;			           		//freq of highest tone, low 493, mid 987, hi 1760
int BUFFER_LEN;
int err;
int PosX;
int PosY;
int inten_frame;

/** Function Header **/
void lightDetect(Mat frame);
void SineWave(int PosX, int PosY);

/** Global Declare **/
snd_output_t *output = NULL;
String window_name_0 = "Original capture";
String window_name_1 = "Light Pointed";
snd_pcm_t *handle;
snd_pcm_sframes_t frames;
//snd_pcm_t *playback_handle;
//snd_pcm_hw_params_t *hw_params;

/**
* @main function
**/
int main(int argc,char const *argv[])
{
    if (argc != 5)
    {
        printf("Invalid argumen!\n");
        printf("-- LightMusic <camera_number> <buffer_length> <low_freq> <hi_freq>\n");
        printf("-- Press Esc to exit\n");
        printf("ex : LightMusic 1 5620 261 1760\n");
        printf("-- <camera_number>  : device number of camera (from 1 to 99)\n");
        printf("-- <buffer_lenght>  : buffer lenght used (from 1000 to 20000)\n");
        printf("-- <low_freq>       : freq of lowest tone, low 261, mid 523, hi 1046\n");
        printf("-- <hi_freq>        : freq of highest tone, low 493, mid 987, hi 1760\n");
        printf("CAUTION!!\n");
        printf("-- bigger number of buffer length, slower frame scan run\n");
        printf("-- smaller number of buffer length, bigger playback sound glitch occur\n");
        printf("-- find right number of buffer length depending on your hardware\n");
        printf("LightMusic -- developed by Lonehack\n");
        return 0;
    }
    int cam = atoi(argv[1]);
    BUFFER_LEN = atoi(argv[2]);
    lotone = atoi(argv[3]);
    hitone = atoi(argv[4]);
	//-- Video prepare
	VideoCapture capture;
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 360);

	Mat frame;
	time_t start, finish;

	//-- Sound error handling
    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printf(" --(!) Playback open error: %s  --\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle,
                                  SND_PCM_FORMAT_FLOAT,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  1,
                                  44100,		//samplerate, standart 44100
                                  1,
                                  80200)) < 0)	//latency, standart 2x samplerate
    {
        printf(" --(!) Playback open error: %s --\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

	//-- Opening video stream
//	for (int cam=1;cam<100;cam++)
//	{
//        capture.open( cam );	//-- opening input : ( -1 ) any camera or camera number (1,...,99), ( argv[1] ) video file
//    }
    capture.open( cam ); //-- opening input : ( -1 ) any camera or camera number (1,...,99), ( argv[1] ) video file

	//-- Checking interface
	if ( ! capture.isOpened() )
	{
		printf("--(!)Error opening video capture --\n");
		return -1;
	}

	//-- Start the clock
    time(&start);
    int counter=0;

	//-- Read captured
	while ( capture.read(frame) )
	{

		if( frame.empty() )
		{
			printf(" --(!) No captured frame -- Break!\n");
			break;
		}

        //-- fix image resolution
        resize(frame, frame, Size(640, 360), 0, 0, INTER_CUBIC);

		//-- Show original frame
		//namedWindow(window_name_0,CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
		//imshow( window_name_0,frame );

		//-- flip frame
        flip(frame, frame, -1);

		//-- Apply the lightDetect
		lightDetect(frame);
		//printf("X = %d, Y = %d, Inten = %d \n", PosX, PosY, inten_frame);

		//-- apply sound parameter
		SineWave(PosX, PosY);

		//Stop the clock and show FPS
        time(&finish);
        counter++;
        double sec=difftime(finish,start);
        double fps=counter/sec;
        printf("fps = %lf\n",fps);

		//-- bail out if escape was pressed
		int c = waitKey(10);
		if( (char)c == 27 )
		{
			printf("\nStoped by User\n");
			break;
		}
	}

	//-- Closing program
	snd_pcm_close(handle);
	capture.release();
	return 0;
}
/**
* @function lightDetect
* By Yoze Rizki
**/
void lightDetect(Mat frame)
{
	Mat frame_gray(frame.size(),frame.type());
	Point highPos;
	int bigest = 0;

	//-- convert to grayscale
	cvtColor(frame,frame_gray,CV_BGR2GRAY);

	//-- detecting light
	for (int y = 0; y < frame_gray.rows; y++)
	{
		for (int x = 0; x < frame_gray.cols; x++)
		{
			inten_frame = frame_gray.at<unsigned char>(Point(x,y));
			if(inten_frame > Thresh)
			{
				if(bigest < inten_frame)
				{
					bigest=inten_frame;
					highPos.x=x;
					highPos.y=y;

				}
			}
		}
	}

	PosX = highPos.x;
	PosY = highPos.y;
	inten_frame = bigest;

	//-- Show Result
	//printf("X = %d,Y =  %d,Inten = %d \n", highPos.x,highPos.y,bigest);
	circle(frame,Point(highPos.x,highPos.y), 5,Scalar(100,0,190), -1);
	flip(frame, frame, 0);
	namedWindow(window_name_1,CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
	imshow( window_name_1,frame );
}
/**
* @function SineWave
**/
void SineWave(int PosX, int PosY)
{
    float buffer [BUFFER_LEN];
	int k, f, ft;
	double amp;
	int fs = BUFFER_LEN; 						//sampling freq
	f = (hitone-lotone)*PosX/640+lotone;		//freq
	if(PosX == 0)
	{
		f = 0;
	}
	amp = 100*PosY/360;							//amplify
	ft = fs*f/44100;
	//-- generating sine wave
	for (k=0; k<BUFFER_LEN; k++)
	{
        buffer[k] = ((amp/100)*sin(2*M_PI*ft/fs*k));
    }
    //-- play sound
    frames = snd_pcm_writei(handle, buffer, BUFFER_LEN);

    //-- show sound play
	printf("\nX = %i, Y = %i \n", PosX, PosY);
	printf("tone %dHz amp %.2f\n",f, amp);
}
