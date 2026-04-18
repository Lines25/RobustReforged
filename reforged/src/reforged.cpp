#include "reforged.h"
#include <chrono>
#include <cstdio>
#include <cstring>

static std::chrono::high_resolution_clock::time_point _tick_start;
static double _last_tick_ms = 0.0;

static constexpr int WINDOW = 20;
static double _tick_history[WINDOW] = {};
static int _tick_index = 0;
static int _tick_count = 0;

static constexpr int MAX_SECTIONS = 1024;

struct Section {
    char name[64];
    std::chrono::high_resolution_clock::time_point start;
    double total_ms;
    int calls;
    bool active;
};

static Section _sections[MAX_SECTIONS] = {};
static int _section_count = 0;

static Section* find_or_create_section(const char* name) {
    for (int i = 0; i < _section_count; i++)
        if (strncmp(_sections[i].name, name, 64) == 0)
            return &_sections[i];
    if (_section_count >= MAX_SECTIONS) return nullptr;
    Section* s = &_sections[_section_count++];
    strncpy(s->name, name, 63);
    s->total_ms = 0.0;
    s->calls = 0;
    s->active = false;
    return s;
}

const char* reforged_hello(void) {
    return "libreforged.so loaded successfully";
}

void reforged_tick_begin(void) {
    _tick_start = std::chrono::high_resolution_clock::now();
}

void reforged_tick_end(void) {
    auto end = std::chrono::high_resolution_clock::now();
    _last_tick_ms = std::chrono::duration<double, std::milli>(end - _tick_start).count();
    _tick_history[_tick_index] = _last_tick_ms;
    _tick_index = (_tick_index + 1) % WINDOW;
    if (_tick_count < WINDOW) _tick_count++;
}

double reforged_tick_last_ms(void) { return _last_tick_ms; }

double reforged_tick_avg_ms(void) {
    if (_tick_count == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < _tick_count; i++) sum += _tick_history[i];
    return sum / _tick_count;
}

int reforged_tick_is_spike(double threshold) {
    if (_tick_count < WINDOW) return 0;
    return (_last_tick_ms - reforged_tick_avg_ms()) > threshold ? 1 : 0;
}

void reforged_section_begin(const char* name) {
    Section* s = find_or_create_section(name);
    if (!s) return;
    s->active = true;
    s->start = std::chrono::high_resolution_clock::now();
}

void reforged_section_end(const char* name) {
    Section* s = find_or_create_section(name);
    if (!s || !s->active) return;
    auto end = std::chrono::high_resolution_clock::now();
    s->total_ms += std::chrono::duration<double, std::milli>(end - s->start).count();
    s->calls++;
    s->active = false;
}

void reforged_sections_report(char* out_buf, int buf_size) {
    int offset = 0;
    for (int i = 0; i < _section_count; i++) {
        Section* s = &_sections[i];
        if (s->calls == 0) continue;
        double avg = s->total_ms / s->calls;
        if (avg >= 0.1) {
	        offset += snprintf(out_buf + offset, buf_size - offset,
	            "  %-20s avg: %6.2fms  total: %8.2fms  calls: %d\n",
	            s->name, avg, s->total_ms, s->calls);
	        if (offset >= buf_size - 1) break;
	    }
    }
}

void reforged_sections_reset(void) {
    for (int i = 0; i < _section_count; i++) {
        _sections[i].total_ms = 0.0;
        _sections[i].calls = 0;
    }
}
