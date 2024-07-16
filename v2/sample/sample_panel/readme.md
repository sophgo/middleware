## sample_panel instructions:

You are advised to use it together with the Screen Interconnection User Guide.

 You can find the corresponding professional terms in the guide

| Command                       | Test project                                                 | Explain                                                      | Example                                   |
| :---------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | :---------------------------------------- |
| ./sample_panel -h             | Display usage                                                | The user quickly obtains the usage of the sample.            | ./sample_panel -h                         |
| ./sample_panel --panel=       | Configure the initialization parameters for the current screen | You can select a screen model based on the screen model you want to test. The supported screen models are displayed in the list below. | ./sample_panel --panel=HX8394_EVB         |
| ./sample_panel --laneid=      | Configure the laneid sequence in sequence                    | Users configure landid based on the sequence of the schematic diagram | ./sample_panel --laneid=1,2,0,3,4         |
| ./sample_panel --pnswap=      | Configure whether the Lane P/N poles of MIPI Tx are switched | The user confirms the Lane polarity of the MIPI Tx according to the screen specification. If inversion is required, 0: no swap, 1: swap. | ./sample_panel --pnswap=0,0,0,0,0         |
| ./sample_panel -d             | Set/get dsi status or Settings                               | Users can set or obtain dsi configurations based on this test item. | ./sample_panel -d                         |
| ./sample_panel  --device=     | Set device number                                            | The user can select the device number as required            | ./sample_panel  --device=0                |
| ./sample_panel --control_pins | Set the screen pin                                           | The user configures the correct pins according to the schematic of the screen, in the order of power, reset, bl_gpio | ./sample_panel --control_pins=399,304,400 |
| ./sample_panel --show-pattern= | Configure the pattern mode | 0: off 1: snow 2: auto 3: red 4: green 5: blue 6: colorbar 7: horizon gray 8: vertical gray 9: black | ./sample_panel --show-pattern=6 |
| ./sample_panel -s        | Show pattern after initialization  | Without any arguements | ./sample_panel -s |



#### ./sample_panel -d  detailed description

```
------------------------dsi-control------------------------
 0: dcs send //Sets the initialization sequence of the screen.
 1: dcs get  //Gets the initialization sequence of the screen.
 2: switch to lp //To switch to LP mode, send or get the initial sequence, you need to 	                   switch to LP mode first.
 3: switch to hs //Switch to HS mode.
 4: get hs settle settings //Get the parameters of hs settle.
 5: set hs settle settings//Set hs settle parameters.
 others: exit

Use example (get screen initialization sequence) :
./sample_panel  --device=1 -d
 others: exit
1
get data size:
1
data type: 0x
0x6
data param: 0x
da
data[0]: 0x83 [1]: 0 [2]: 0 [3]: 0
```

#### Supported screen models：

```
optional panel mode support list:
 3AML069LP01G
 GM8775C
 HX8394_EVB
 HX8399_1080P
 ICN9707
 ILI9881C
 ILI9881D
 JD9366AB
 LT9611_1920x1080_60
 LT9611_1920x1080_30
 LT9611_1280x720_60
 LT9611_1024x768_60
 LT9611_1280x1024_60
 LT9611_1600x1200_60
 LT9611_2560x1440_60
 NT35521
 OTA7290B_1920
 OTA7290B
 ST7701
 LCM185X56
```

