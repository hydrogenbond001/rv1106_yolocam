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



void setup_scr_Demo(lv_ui *ui)
{
    //Write codes Demo
    ui->Demo = lv_obj_create(NULL);
    lv_obj_set_size(ui->Demo, 480, 480);
    lv_obj_set_scrollbar_mode(ui->Demo, LV_SCROLLBAR_MODE_OFF);

    //Write style for Demo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Demo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Demo, lv_color_hex(0x98efd6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Demo, LV_GRAD_DIR_HOR, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->Demo, lv_color_hex(0x5fdff7), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui->Demo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui->Demo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Demo_tabview_1
    ui->Demo_tabview_1 = lv_tabview_create(ui->Demo, LV_DIR_TOP, 50);
    lv_obj_set_pos(ui->Demo_tabview_1, 5, 1);
    lv_obj_set_size(ui->Demo_tabview_1, 480, 480);
    lv_obj_set_scrollbar_mode(ui->Demo_tabview_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for Demo_tabview_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Demo_tabview_1, 218, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Demo_tabview_1, lv_color_hex(0xeaeff3), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Demo_tabview_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Demo_tabview_1, lv_color_hex(0x4d4d4d), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Demo_tabview_1, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Demo_tabview_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Demo_tabview_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Demo_tabview_1, 16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->Demo_tabview_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Demo_tabview_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Demo_tabview_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_Demo_tabview_1_extra_btnm_main_default
    static lv_style_t style_Demo_tabview_1_extra_btnm_main_default;
    ui_init_style(&style_Demo_tabview_1_extra_btnm_main_default);

    lv_style_set_bg_opa(&style_Demo_tabview_1_extra_btnm_main_default, 255);
    lv_style_set_bg_color(&style_Demo_tabview_1_extra_btnm_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_Demo_tabview_1_extra_btnm_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_border_width(&style_Demo_tabview_1_extra_btnm_main_default, 0);
    lv_style_set_radius(&style_Demo_tabview_1_extra_btnm_main_default, 0);
    lv_obj_add_style(lv_tabview_get_tab_btns(ui->Demo_tabview_1), &style_Demo_tabview_1_extra_btnm_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_Demo_tabview_1_extra_btnm_items_default
    static lv_style_t style_Demo_tabview_1_extra_btnm_items_default;
    ui_init_style(&style_Demo_tabview_1_extra_btnm_items_default);

    lv_style_set_text_color(&style_Demo_tabview_1_extra_btnm_items_default, lv_color_hex(0x4d4d4d));
    lv_style_set_text_font(&style_Demo_tabview_1_extra_btnm_items_default, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_Demo_tabview_1_extra_btnm_items_default, 255);
    lv_obj_add_style(lv_tabview_get_tab_btns(ui->Demo_tabview_1), &style_Demo_tabview_1_extra_btnm_items_default, LV_PART_ITEMS|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_CHECKED for &style_Demo_tabview_1_extra_btnm_items_checked
    static lv_style_t style_Demo_tabview_1_extra_btnm_items_checked;
    ui_init_style(&style_Demo_tabview_1_extra_btnm_items_checked);

    lv_style_set_text_color(&style_Demo_tabview_1_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_text_font(&style_Demo_tabview_1_extra_btnm_items_checked, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_Demo_tabview_1_extra_btnm_items_checked, 255);
    lv_style_set_border_width(&style_Demo_tabview_1_extra_btnm_items_checked, 4);
    lv_style_set_border_opa(&style_Demo_tabview_1_extra_btnm_items_checked, 255);
    lv_style_set_border_color(&style_Demo_tabview_1_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_border_side(&style_Demo_tabview_1_extra_btnm_items_checked, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_radius(&style_Demo_tabview_1_extra_btnm_items_checked, 0);
    lv_style_set_bg_opa(&style_Demo_tabview_1_extra_btnm_items_checked, 60);
    lv_style_set_bg_color(&style_Demo_tabview_1_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_bg_grad_dir(&style_Demo_tabview_1_extra_btnm_items_checked, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_tabview_get_tab_btns(ui->Demo_tabview_1), &style_Demo_tabview_1_extra_btnm_items_checked, LV_PART_ITEMS|LV_STATE_CHECKED);

    //Write codes tab1
    ui->Demo_tabview_1_tab_1 = lv_tabview_add_tab(ui->Demo_tabview_1,"tab1");
    lv_obj_t * Demo_tabview_1_tab_1_label = lv_label_create(ui->Demo_tabview_1_tab_1);
    lv_label_set_text(Demo_tabview_1_tab_1_label, "con1");

    //Write codes Demo_img_1
    ui->Demo_img_1 = lv_img_create(ui->Demo_tabview_1_tab_1);
    lv_obj_add_flag(ui->Demo_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->Demo_img_1, &_127416063_p0_alpha_473x703);
    lv_img_set_pivot(ui->Demo_img_1, 50,50);
    lv_img_set_angle(ui->Demo_img_1, 0);
    lv_obj_set_pos(ui->Demo_img_1, -19, -31);
    lv_obj_set_size(ui->Demo_img_1, 473, 703);

    //Write style for Demo_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->Demo_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->Demo_img_1, 120, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Demo_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->Demo_img_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes tab2
    ui->Demo_tabview_1_tab_2 = lv_tabview_add_tab(ui->Demo_tabview_1,"tab2");
    lv_obj_t * Demo_tabview_1_tab_2_label = lv_label_create(ui->Demo_tabview_1_tab_2);
    lv_label_set_text(Demo_tabview_1_tab_2_label, "con2");

    //Write codes Demo_sw_1
    ui->Demo_sw_1 = lv_switch_create(ui->Demo_tabview_1_tab_2);
    lv_obj_set_pos(ui->Demo_sw_1, 326, 82);
    lv_obj_set_size(ui->Demo_sw_1, 40, 20);

    //Write style for Demo_sw_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Demo_sw_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Demo_sw_1, lv_color_hex(0xe6e2e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Demo_sw_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->Demo_sw_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Demo_sw_1, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Demo_sw_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for Demo_sw_1, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->Demo_sw_1, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->Demo_sw_1, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->Demo_sw_1, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->Demo_sw_1, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for Demo_sw_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Demo_sw_1, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Demo_sw_1, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Demo_sw_1, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->Demo_sw_1, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Demo_sw_1, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes Demo_lottie_2
    ui->Demo_lottie_2 = lv_rlottie_create_from_raw(ui->Demo_tabview_1_tab_2, 100, 100, (const void *)lottie_lottieflow_loading);
    lv_obj_set_pos(ui->Demo_lottie_2, 147, 175);
    lv_obj_set_size(ui->Demo_lottie_2, 100, 100);

    //Write style for Demo_lottie_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Demo_lottie_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Demo_qrcode_1
    ui->Demo_qrcode_1 = lv_qrcode_create(ui->Demo_tabview_1_tab_2, 100, lv_color_hex(0x2C3224), lv_color_hex(0xffffff));
    const char * Demo_qrcode_1_data = "https://www.nxp.com/";
    lv_qrcode_update(ui->Demo_qrcode_1, Demo_qrcode_1_data, 20);
    lv_obj_set_pos(ui->Demo_qrcode_1, 147, 47);
    lv_obj_set_size(ui->Demo_qrcode_1, 100, 100);

    //Write codes tab3
    ui->Demo_tabview_1_tab_3 = lv_tabview_add_tab(ui->Demo_tabview_1,"tab3");
    lv_obj_t * Demo_tabview_1_tab_3_label = lv_label_create(ui->Demo_tabview_1_tab_3);
    lv_label_set_text(Demo_tabview_1_tab_3_label, "con3");

    //Write codes Demo_lottie_1
    ui->Demo_lottie_1 = lv_rlottie_create_from_raw(ui->Demo, 100, 100, "");
    lv_obj_set_pos(ui->Demo_lottie_1, -213, 216);
    lv_obj_set_size(ui->Demo_lottie_1, 100, 100);

    //Write style for Demo_lottie_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Demo_lottie_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of Demo.


    //Update current screen layout.
    lv_obj_update_layout(ui->Demo);

}
