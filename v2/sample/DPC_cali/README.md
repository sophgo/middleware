# Use dpc_cali

The dpc_cali tool can be used for DPC calibration to detect bright and dark dead pixels in images in either online or offline mode, and saves their coordinates to a file. 

# Notice:

Before using dpc_cali, please prepare the sensor as follows:

For detecting bright dead pixels: **Completely cover the lens with black tape** to prevent any external light from affecting the sensor, then proceed with DPC calibration.

For detecting dark dead pixels: **Place the lens in front of a light box, turn the brightness of the light box to maximum**, and then start the DPC calibration program.

## Usage:

./dpc_cali <mode> <threshold> <decttype> <input_raw_path/bright_pixel_txt> <output_table_path/dark_pixel_txt> (<nbit> <width> <height>)

## Index:

<mode>:
        0: offline mode
        1: online mode
<threshold>:
        the absolute threshold value
<decttype>:
        0: detect bright pixel in dark raws
        1: detect dark pixel in birght raws
<input_raw_path/bright_pixel_txt>:
         the raw dir path in offline mode
         the brigth table output path in online mode
<output_table_path/dark_pixel_txt>:
        the output table path in offline mode
        the dark table output path in online mode
<nbit>:
        the positive pixel bit number in offline mode, only support 12 or 16 bits
<width>:
        the positive width of raw file in offline mode
<height>:
        the positive height of raw file in offline mode

## Examples

Online  eg :    ./dpc_cali 1 1 0 ./bright110.txt ./dark.txt
           :    ./dpc_cali 1 5 1 ./bright.txt ./dark151.txt

Offline eg :    ./dpc_cali 0 1 0 ./inputRaw ./bright.txt 16 2560 1440
           :    ./dpc_cali 0 1 1 ./inputRaw ./dark.txt 16 2560 1440
