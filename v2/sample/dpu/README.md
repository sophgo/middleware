#DPU SAMPLE

#case 0: output a u8 sgbm disparity image(no post process)
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.do ./sample_dpu 0

#case 1: output a u16 sgbm disparity image(via post process)
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.do ./sample_dpu 1

#case 2: output a u8 sgbm disparity image(via post process)
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.do ./sample_dpu 2

#case 4: output a u8 sgbm disparity image(via fgs)
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.do ./sample_dpu 4

#case 5: output a u16 sgbm depth image(via fgs)
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.do ./sample_dpu 5

#case 6: output a u16 sgbm depth image
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.do ./sample_dpu 6

#case 7: output a u8 fgs image
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.do ./sample_dpu 7

#case 8: output a u16 fgs depth image
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.do ./sample_dpu 8

#case 9: output a u8 sgbm depth image(via dwa rectify)
1.prepare two pictures(named Teddy_left_img.bin and Teddy_right_img.bin),width is 448,height is 368.
2.prepare two  gridinfo bins(named grid_info_27_22_594_28_23_448x368_L.dat and grid_info_27_22_594_28_23_448x368_R.dat)
3.do ./sample_dpu 9

note:
the two pictures and two gridinfo bins can be found in "https://disk.sophgo.vip/ multimedia/A2/auto_test/res/dpu/dpu_demo/"