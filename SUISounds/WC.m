## Author: Ramesh Yerraballi
## Use this in octave to convert a wav file into a C declaration of the
## the samples at 12-bit precision and 11.025kHz
## You will need to load the signal package first to use  the decimate function
## You can load it like this on the octave prompt:
##    pkg load signal 
## Run in octave as:
##    WC ('filename'); 
## Optional second argument (e.g. WC('filename', 8)) is accepted for compatibility but ignored.
## The input is the name of the wav file without the extention .wav.
## This will generate a filename.txt file that has a C declaration:
##    const uint16_t  filename[N] = { ... };   /* 12-bit codes for MCU DAC */
## For example, say you have a sound file called boom.wav that you want to use in
## in your game. 
##    1. Put the WC.m script and the boom.wav file in a folder (say sounds).
##    2. Open octave/matlab and cd to the sounds folder
##    3. Run script as: WC("boom")
##       This will produce a file named boom.txt, which contains a C declaration
##       for the array of samples. You can cut-and-paste it in your code
function WC(filename,varargin)
  # load the file
  [SplsOrig, fs] = audioread(strcat(filename,'.wav')); 
  # downsample to 11.025kHz
  Spls = decimate(SplsOrig,round(fs/11025)); 
  # trim to 12-bit precision for MCU DAC
  Spls = round((Spls+ 1)* (2^12-1)/2);
  # write C declaration to file with txt extension  
  file = fopen(strcat(filename,'.txt'), 'w');          
  fprintf(file, 'const uint16_t\t%s[%d] = {', filename, length(Spls));
  # The sample dump is done here
  fprintf(file, '%d,', Spls(1:length(Spls)-1));        
  fprintf(file, '%d', Spls(length(Spls)));
  fprintf(file, '};\n');
  fclose(file);
end
