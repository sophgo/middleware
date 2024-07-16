# HDMI Sample Application


## Overview
This HDMI Sample Application demonstrates how to configure HDMI settings, including video format, audio settings, and EDID information. It is designed to run on CVITEK platform.


## Prerequisites
- A supported development board with HDMI output.
- The SDK installed and properly configured.


## Compilation
To compile the HDMI Sample Application, you can use the provided Makefile and SDK.


## Usage
To run the HDMI Sample Application, use the following command:

./sample_hdmi <mCode> <pixel_clk> <force_output> <pixel_repeat> <hdcp14_en> <csc_en> <audio_en> <csc_fmt_in> <csc_fmt_out> <avmute_en> <audio_mute_en> <set_infoframe> <exit_flag> <audio_file>


## Parameters
| Parameter        | Description                                       |
|------------------|---------------------------------------------------|
| mCode            | Mode code corresponding to the video format.      |
| pixel_clk        | Pixel clock frequency.                            |
| force_output     | Force output even if not supported by the sink.   |
| pixel_repeat     | Pixel repetition factor.                          |
| hdcp14_en        | Enable HDCP 1.4.                                  |
| csc_en           | Enable color space conversion.                    |
| audio_en         | Enable audio output.                              |
| csc_fmt_in       | Input format for color space conversion.          |
| csc_fmt_out      | Output format for color space conversion.         |
| avmute_en        | Enable AVMUTE.                                    |
| audio_mute_en    | Mute audio output.                                |
| set_infoframe    | Set HDMI infoframe.                               |
| exit_flag        | Control the end of this process                   |
| audio_file       | Path to the audio file to be played.              |

| mCode | Resolution     | Refresh Rate |
|-------|----------------|--------------|
| 1     | 640x480p       | 60           |
| 2     | 720x480p       | 60           |
| 4     | 1280x720p      | 60           |
| 16    | 1920x1080p     | 60           |
| 90    | 2560x1080p     | 60           |
| 102   | 4096x2160p     | 60           |
| 107   | 3840x2160p     | 60           |
| 108   | 2560x1440p     | 60           |

| pic_fmt | Color Space |
|---------|-------------|
| 0       | RGB888      |
| 1       | YUV444      |
| 2       | YUV422      |

Note: If you are testing pixel repeat, you should set set_infoframe to 1.
      PCM audio format only supports iec60958.
      loopback test should set exit_flag = 0.
      In addition, just set exit_flag = 1.

## Example
./sample_hdmi --mcode 16 --pixel_clk 148500 --force_output 0 --pixel_repeat 0 --hdcp14_en 0 --csc_en 0 --audio_en 0 --fmt_in 0 --fmt_out 0 --avmute 0 --audio_mute 0 --set_infoframe 0 --exit_flag 1 ./audio.file

Execute the above line of commands to output 1920x1080-60Hz images.
If you have other needs, you can refer to the parameter list above to set it yourself.


## Features
HDMI interface initialization and de-initialization.
Setting HDMI attributes including video and audio parameters.
Retrieving and displaying HDMI EDID information.
Getting and printing HDMI sink capabilities.
Handling HDMI events through a callback mechanism.