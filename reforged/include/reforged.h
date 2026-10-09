#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const char* reforged_hello(void);

void reforged_tick_begin(void);
void reforged_tick_end(void);
double reforged_tick_last_ms(void);
double reforged_tick_avg_ms(void);
int reforged_tick_is_spike(double threshold);

void reforged_section_begin(const char* name);
void reforged_section_end(const char* name);
void reforged_sections_report(char* out_buf, int buf_size);
void reforged_sections_reset(void);

#ifdef __cplusplus
}
#endif
