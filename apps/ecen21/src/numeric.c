/* Masking, two's-complement magnitude and reverse-digit formatting conventions
 * adapted from apps/base-n/src/main.c: Copyright (c)2026 José Rodrigues, MIT.
 * Parsing, explicit errors, arithmetic flags and bounded model are new. */
#include "numeric.h"
#include <stdio.h>
#include <string.h>

static bool valid_width(unsigned width)
{ return width >= 1 && width <= 32; }

static bool valid_base(unsigned base)
{ return base == 2 || base == 10 || base == 16; }

uint32_t nm_mask(unsigned width)
{
    if(!valid_width(width)) return 0;
    return width == 32 ? UINT32_MAX : (UINT32_C(1) << width) - 1;
}

int64_t nm_signed_value(uint32_t value, unsigned width)
{
    if(!valid_width(width)) return 0;
    value &= nm_mask(width);
    return (int64_t)value - ((value & (UINT32_C(1) << (width - 1))) ?
        (INT64_C(1) << width) : 0);
}

int nm_format(uint32_t value, unsigned width, unsigned base,
    bool signed_decimal, char *buffer, size_t capacity)
{
    if(!valid_width(width) || !valid_base(base) || (!buffer && capacity)) return -1;
    value &= nm_mask(width);
    int64_t interpreted = nm_signed_value(value, width);
    bool negative = base == 10 && signed_decimal && interpreted < 0;
    uint64_t magnitude = negative ? (uint64_t)-interpreted : value;
    char temporary[35], *end = temporary + sizeof temporary, *p = end;
    unsigned minimum = base == 2 ? width : base == 16 ? (width + 3) / 4 : 1;
    static char const digits[] = "0123456789ABCDEF";
    do {
        *--p = digits[magnitude % base];
        magnitude /= base;
    } while(magnitude || (unsigned)(end - p) < minimum);
    if(negative) *--p = '-';
    size_t length = (size_t)(end - p);
    if(capacity) {
        size_t copied = length < capacity - 1 ? length : capacity - 1;
        memcpy(buffer, p, copied);
        buffer[copied] = '\0';
    }
    return (int)length;
}

static void clear_result(nm_state *s)
{
    s->solved = false;
    s->result = 0;
    s->carry = s->borrow = s->overflow = s->wrapped = false;
}

static bool error(nm_state *s, const char *message)
{
    clear_result(s);
    snprintf(s->error, sizeof s->error, "%s", message);
    return false;
}

static bool input_error(nm_state *s, unsigned operand, const char *message)
{
    clear_result(s);
    s->operand_valid[operand] = false;
    s->operand[operand] = 0;
    snprintf(s->error, sizeof s->error, "%c: %s", 'A' + (int)operand, message);
    return false;
}

static int digit_value(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static bool parse_input(nm_state *s, unsigned operand)
{
    clear_result(s);
    const char *text = s->input[operand];
    size_t length = strlen(text), start = 0;
    bool negative = false;
    if(!valid_width(s->width) || !valid_base(s->base))
        return input_error(s, operand, "Unsupported width/base");
    if(!length) return input_error(s, operand, "Enter a value");
    if(text[0] == '-') {
        if(s->base != 10 || !s->signed_mode)
            return input_error(s, operand, "Minus requires signed DEC");
        negative = true; start = 1;
        if(length == 1) return input_error(s, operand, "Enter digits after minus");
    }
    if((s->base == 2 && length > s->width) ||
       (s->base == 16 && length > (s->width + 3) / 4))
        return input_error(s, operand, "Too many digits for width");
    uint64_t limit = nm_mask(s->width), magnitude = 0;
    if(s->base == 10 && s->signed_mode)
        limit = (UINT64_C(1) << (s->width - 1)) - (negative ? 0 : 1);
    for(size_t i = start; i < length; i++) {
        int d = digit_value(text[i]);
        if(d < 0 || (unsigned)d >= s->base)
            return input_error(s, operand, "Invalid digit for selected base");
        if((unsigned)d > limit || magnitude > (limit - (unsigned)d) / s->base)
            return input_error(s, operand, "Outside selected numeric range");
        magnitude = magnitude * s->base + (unsigned)d;
    }
    s->operand[operand] = (negative ? UINT32_C(0) - (uint32_t)magnitude :
        (uint32_t)magnitude) & nm_mask(s->width);
    s->operand_valid[operand] = true;
    s->truncated = false;
    if(s->operand_valid[1 - operand]) s->error[0] = '\0';
    else snprintf(s->error, sizeof s->error, "%c: fix invalid input", 'A' + (int)(1 - operand));
    return true;
}

void nm_init(nm_state *s)
{
    if(!s) return;
    memset(s, 0, sizeof *s);
    s->width = 8; s->base = 10; s->operation = NM_ADD;
    nm_set_input(s, 0, "255"); nm_set_input(s, 1, "1");
}

bool nm_set_input(nm_state *s, unsigned operand, const char *text)
{
    if(!s) return false;
    if(operand > 1) return error(s, "Invalid operand selection");
    if(!text) return input_error(s, operand, "Missing input text");
    size_t length = 0;
    while(length < sizeof s->input[operand] && text[length]) length++;
    if(length == sizeof s->input[operand]) {
        memmove(s->input[operand], text, sizeof s->input[operand] - 1);
        s->input[operand][sizeof s->input[operand] - 1] = '\0';
        return input_error(s, operand, "Input exceeds34 characters");
    }
    memmove(s->input[operand], text, length + 1);
    return parse_input(s, operand);
}

bool nm_append(nm_state *s, unsigned operand, char character)
{
    if(!s) return false;
    if(operand > 1) return error(s, "Invalid operand selection");
    if(character < ' ' || character > '~')
        return input_error(s, operand, "Invalid input character");
    size_t length = strlen(s->input[operand]);
    if(length >= sizeof s->input[operand] - 1)
        return input_error(s, operand, "Input exceeds34 characters");
    s->input[operand][length] = character;
    s->input[operand][length + 1] = '\0';
    return parse_input(s, operand);
}

void nm_backspace(nm_state *s, unsigned operand)
{
    if(!s) return;
    if(operand > 1) { error(s, "Invalid operand selection"); return; }
    size_t length = strlen(s->input[operand]);
    if(length) s->input[operand][length - 1] = '\0';
    parse_input(s, operand);
}

void nm_clear(nm_state *s, unsigned operand)
{
    if(!s) return;
    if(operand > 1) { error(s, "Invalid operand selection"); return; }
    s->input[operand][0] = '\0';
    input_error(s, operand, "Enter a value");
}

static bool ready_for_setting(nm_state *s)
{
    if(!s->operand_valid[0] || !s->operand_valid[1])
        return error(s, "Fix both operands before settings");
    if(!valid_width(s->width) || !valid_base(s->base))
        return error(s, "Unsupported width/base");
    return true;
}

static void reformat(nm_state *s)
{
    for(unsigned i = 0; i < 2; i++)
        nm_format(s->operand[i], s->width, s->base, s->signed_mode,
            s->input[i], sizeof s->input[i]);
    clear_result(s); s->error[0] = '\0';
}

bool nm_set_base(nm_state *s, unsigned base)
{
    if(!s) return false;
    if(!valid_base(base)) return error(s, "Choose BIN, DEC or HEX");
    if(!ready_for_setting(s)) return false;
    if(base != s->base) s->truncated = false;
    s->base = base; reformat(s);
    return true;
}

bool nm_set_signed(nm_state *s, bool signed_mode)
{
    if(!s) return false;
    if(!ready_for_setting(s)) return false;
    if(signed_mode != s->signed_mode) s->truncated = false;
    s->signed_mode = signed_mode; reformat(s);
    return true;
}

bool nm_set_width(nm_state *s, unsigned width)
{
    if(!s) return false;
    if(!valid_width(width)) return error(s, "Choose1..32 bits");
    if(!ready_for_setting(s)) return false;
    unsigned old = s->width;
    if(width == old) { reformat(s); return true; }
    s->truncated = false;
    for(unsigned i = 0; i < 2; i++) {
        uint32_t previous = s->operand[i] & nm_mask(old), next = previous;
        if(width > old && s->signed_mode && nm_signed_value(previous, old) < 0)
            next |= ~nm_mask(old);
        next &= nm_mask(width);
        if(width < old && (s->signed_mode ?
            nm_signed_value(previous, old) != nm_signed_value(next, width) : previous != next))
            s->truncated = true;
        s->operand[i] = next;
    }
    s->width = width; reformat(s);
    return true;
}

void nm_set_operation(nm_state *s, nm_operation operation)
{
    if(!s) return;
    if(operation != NM_ADD && operation != NM_SUB) {
        error(s, "Choose addition or subtraction"); return;
    }
    s->operation = operation; clear_result(s);
    if(s->operand_valid[0] && s->operand_valid[1]) s->error[0] = '\0';
}

bool nm_solve(nm_state *s)
{
    if(!s) return false;
    clear_result(s);
    if(!ready_for_setting(s)) return false;
    if(s->operation != NM_ADD && s->operation != NM_SUB)
        return error(s, "Choose addition or subtraction");
    uint32_t mask = nm_mask(s->width);
    uint32_t a = s->operand[0], b = s->operand[1];
    if((a | b) & ~mask) return error(s, "Operand bits exceed selected width");
    uint32_t sign = UINT32_C(1) << (s->width - 1);
    if(s->operation == NM_ADD) {
        uint64_t sum = (uint64_t)a + b;
        s->result = (uint32_t)sum & mask;
        s->carry = sum > mask;
        s->overflow = ((~(a ^ b) & (a ^ s->result) & sign) != 0);
    }
    else {
        s->result = (a - b) & mask;
        s->borrow = a < b;
        s->overflow = (((a ^ b) & (a ^ s->result) & sign) != 0);
    }
    s->wrapped = s->signed_mode ? s->overflow : s->carry || s->borrow;
    s->solved = true; s->error[0] = '\0';
    return true;
}
