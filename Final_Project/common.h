// common.h - 放在所有人的資料夾中
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

// 定義一個全域變數來計算步數
// 使用 extern 關鍵字，表示這個變數是在 main.c 裡面真正被宣告的
extern unsigned long long global_step_count;

// 定義一個增加步數的巨集或函式，讓 A 和 B 在移動時呼叫
// 這樣你的主程式才能統計到他們的動作
void increment_step();

#endif