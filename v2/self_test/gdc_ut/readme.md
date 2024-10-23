## gdc ut instructions



| ut                             | usage                  | purpose                                                              |
| ------------------------------ | -----------------------| -------------------------------------------------------------------- |
| test rot                       | ./gdc_ut 0 1           | Implement the rotation                                               |
| test rot small                 | ./gdc_ut 1 1           | Implement the small size rotation                                    |
| test rot 4m                    | ./gdc_ut 2 1           | Implement the 4m size rotation                                       |
| test ldc                       | ./gdc_ut 3 1           | Implement the ldc                                                    |
| test maxsize                   | ./gdc_ut 4 1           | Implement the max size ldc                                           |
| test fisheye                   | ./gdc_ut 5 1           | Implement the fisheye                                                |
| test affine                    | ./gdc_ut 6 1           | Implement the affine                                                 |
| test fmt                       | ./gdc_ut 7 1           | Implement the fmt test                                               |
| test size not align            | ./gdc_ut 8 1           | Implement the size not align test                                    |
| test cmdq                      | ./gdc_ut 9 1           | Implement the cmdq                                                   |
| test cmdq_1to2                 | ./gdc_ut 10 1          | Implement the cmdq_1to2                                              |
| test cmdq_1to2 maxsize         | ./gdc_ut 11 1          | Implement the cmdq_1to2 maxsize                                      |
| test online                    | ./gdc_ut 12 1          | Implement the online                                                 |
| test async                     | ./gdc_ut 13 1          | Implement the async                                                  |
| test mix                       | ./gdc_ut 14 1          | Implement the mix job                                                |
| test mix fasync                | ./gdc_ut 15 1          | Implement the fasync to get frame                                    |
| test multi thread              | ./gdc_ut 16 1          | Implement the multi thread                                           |
| test pef                       | ./gdc_ut 17 1          | Implement the pef test                                               |
| test grid_info_ldc             | ./gdc_ut 18 1          | Implement the load ldc gridinfo                                      |
| test grid_info_fisheye         | ./gdc_ut 19 1          | Implement the load fisheye gridinfo                                  |
| test grid_info_dewarp          | ./gdc_ut 20 1          | Implement the load dewarp gridinfo                                   |
| test reset                     | ./gdc_ut 21 1          | Implement the reset                                                  |
| test suspend                   | ./gdc_ut 22 1          | Implement the suspend                                                |
| test resume                    | ./gdc_ut 23 1          | Implement the resume                                                 |
| test running suspend           | ./gdc_ut 24 1          | Implement the running suspend                                        |
| test dis                       | ./gdc_ut 25 1          | Implement the dis                                                    |
| test presure size for each     | ./gdc_ut 98 1          | Implement the presure size for each                                  |
| test auto regression           | ./gdc_ut 99 1          | Implement the auto regression                                        |
| user cofig test                | ./gdc_ut 100 1         | Implement the user cofig                                             |
| gdc dup fd                     | ./gdc_ut 101 1         | Implement the gdc dup fd                                             |
| gdc rst fd                     | ./gdc_ut 102 1         | Implement the gdc rst fd                                             |
|                                |                        |                                                                      |

Parameter 0: ut file
Parameter 1: case number
Parameter 2: 1:save output file, 0:no save output file