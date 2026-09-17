#include "bsp_flash.h"
#include "app_data.h"
#include <string.h>

// ========== 保存数据结构（严格 4 字节对齐） ==========
// 注意：结构体字段全部用 uint32_t，保证 4 字节对齐
typedef struct {
    uint32_t checksum;        // 校验和放在第一位！
    uint32_t magic;           // 魔数
    uint32_t temp_min;        // 阈值数据（用定点数表示，如 23.0℃ 存为 230）
    uint32_t temp_max;
    uint32_t humi_min;
    uint32_t humi_max;
    uint32_t light_min;
    uint32_t light_max;
    uint32_t soil_min;
    uint32_t soil_max;
    uint32_t reserved;        // 保留位，保证总长度是 4 的倍数
} FlashData_t;

// ========== 简单校验和（所有字节累加） ==========
static uint32_t CalcChecksum(uint8_t *data, uint32_t len)
{
    uint32_t sum = 0;
    uint32_t i;
    for (i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// ========== 保存阈值到 Flash ==========
void Flash_SaveThreshold(void)
{
    FlashData_t data;
    uint32_t *p;
    uint32_t i;
    uint32_t word_count;
    
    // 1. 填充数据（float 阈值转成定点数存储，避免浮点对齐问题）
    data.magic     = 0xA5A5A5A5;
    data.temp_min  = (uint32_t)(g_threshold.temp_min * 10);   // 23.5 → 235
    data.temp_max  = (uint32_t)(g_threshold.temp_max * 10);
    data.humi_min  = (uint32_t)(g_threshold.humi_min * 10);
    data.humi_max  = (uint32_t)(g_threshold.humi_max * 10);
    data.light_min = g_threshold.light_min;
    data.light_max = g_threshold.light_max;
    data.soil_min  = g_threshold.soil_min;
    data.soil_max  = g_threshold.soil_max;
    data.reserved  = 0;
    
    // 2. 计算校验和（从 magic 开始，到结构体末尾）
    data.checksum = CalcChecksum((uint8_t*)&data + 4, sizeof(data) - 4);
    
    // 3. 解锁 Flash
    FLASH_Unlock();
    
    // 4. 擦除最后一页
    FLASH_ErasePage(FLASH_SAVE_ADDR);
    
    // 5. 强制 4 字节对齐写入（关键！）
    word_count = (sizeof(data) + 3) / 4;   // 向上取整到 4 字节
    p = (uint32_t*)&data;
    for (i = 0; i < word_count; i++) {
        FLASH_ProgramWord(FLASH_SAVE_ADDR + i * 4, p[i]);
    }
    
    // 6. 锁定 Flash
    FLASH_Lock();
}

// ========== 从 Flash 读取阈值 ==========
void Flash_LoadThreshold(void)
{
    FlashData_t *p = (FlashData_t*)FLASH_SAVE_ADDR;
    uint32_t calc_checksum;
    
    // 1. ? 先检查校验位（放在第一位，第一时间判断数据有效性）
    if (p->magic != 0xA5A5A5A5) {
        // 魔数不对，说明从来没保存过，用默认值
        Data_Init();
        return;
    }
    
    // 2. 校验和验证
    calc_checksum = CalcChecksum((uint8_t*)p + 4, sizeof(FlashData_t) - 4);
    if (calc_checksum != p->checksum) {
        // 校验失败，数据损坏，用默认值
        Flash_LoadThreshold();   // 上电从 Flash 读取
        return;
    }
    
    // 3. 读取成功，把定点数还原成 float
    g_threshold.temp_min  = p->temp_min  / 10.0f;
    g_threshold.temp_max  = p->temp_max  / 10.0f;
    g_threshold.humi_min  = p->humi_min  / 10.0f;
    g_threshold.humi_max  = p->humi_max  / 10.0f;
    g_threshold.light_min = p->light_min;
    g_threshold.light_max = p->light_max;
    g_threshold.soil_min  = p->soil_min;
    g_threshold.soil_max  = p->soil_max;
}
