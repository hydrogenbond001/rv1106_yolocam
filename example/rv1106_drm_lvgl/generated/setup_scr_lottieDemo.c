/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_lottieDemo(lv_ui *ui)
{
    //Write codes lottieDemo
    ui->lottieDemo = lv_obj_create(NULL);
    lv_obj_set_size(ui->lottieDemo, 480, 480);
    lv_obj_set_scrollbar_mode(ui->lottieDemo, LV_SCROLLBAR_MODE_OFF);

    //Write style for lottieDemo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->lottieDemo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->lottieDemo, lv_color_hex(0x98efd6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->lottieDemo, LV_GRAD_DIR_HOR, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->lottieDemo, lv_color_hex(0x5fdff7), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui->lottieDemo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui->lottieDemo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes lottieDemo_img_1
    ui->lottieDemo_img_1 = lv_img_create(ui->lottieDemo);
    lv_obj_add_flag(ui->lottieDemo_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->lottieDemo_img_1, &_127416063_p0_alpha_314x470);
    lv_img_set_pivot(ui->lottieDemo_img_1, 50,50);
    lv_img_set_angle(ui->lottieDemo_img_1, 0);
    lv_obj_set_pos(ui->lottieDemo_img_1, 7, 6);
    lv_obj_set_size(ui->lottieDemo_img_1, 314, 470);

    //Write style for lottieDemo_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->lottieDemo_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->lottieDemo_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->lottieDemo_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->lottieDemo_img_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of lottieDemo.


    //Update current screen layout.
    lv_obj_update_layout(ui->lottieDemo);

}
