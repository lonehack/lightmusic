# lightmusic
Play sound from detected light

A simple C++ code to locate brightest pixel for frequency and amplitude
and play sound with OpenCV 2.4 and libasound.

Command: LightMusic <camera_number> <buffer_length> <low_freq> <hi_freq>
ex : LightMusic 1 5620 261 1760
<camera_number>  : device number of camera (from 1 to 99)
<buffer_lenght>  : buffer lenght used (from 1000 to 20000)
<low_freq>       : freq of lowest tone, low 261, mid 523, hi 1046
<hi_freq>        : freq of highest tone, low 493, mid 987, hi 1760

bigger number of buffer length, slower frame scan run
smaller number of buffer length, bigger playback sound glitch occur
find right number of buffer length depending on your device

Compile :
      g++ lightmusic.cpp -lasound -o lightmusic `pkg-config --cflags --libs opencv`
