/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef ML_LJ92_PREVIEW_H
#define ML_LJ92_PREVIEW_H

/* File-backed lossless JPEG for the playback modules. A predictor row is
 * enough to decode straight into the renderer's native RAW layout. */
struct lj92_table {
    uint16_t fast[256];
    uint8_t symbols[256];
    int first[16], count[16], start[16];
    int defined;
};

struct lj92_stream {
    FILE *file;
    unsigned remaining, pos, end;
    uint32_t bits;
    int available, marker;
    uint8_t bytes[8192];
    struct lj92_table tables[4];
};

static int lj92_byte(struct lj92_stream *s)
{
    if (s->pos == s->end)
    {
        if (!s->remaining) return -1;
        unsigned n = MIN(s->remaining, sizeof(s->bytes));
        if (FIO_ReadFile(s->file, s->bytes, n) != (int)n)
        {
            s->remaining = 0;
            return -1;
        }
        s->remaining -= n;
        s->pos = 0;
        s->end = n;
    }
    return s->bytes[s->pos++];
}

static int lj92_word(struct lj92_stream *s)
{
    int hi = lj92_byte(s);
    int lo = lj92_byte(s);
    return (hi < 0 || lo < 0) ? -1 : (hi << 8) | lo;
}

static int lj92_marker(struct lj92_stream *s)
{
    if (lj92_byte(s) != 255) return -1;
    int marker;
    do { marker = lj92_byte(s); } while (marker == 255);
    return marker;
}

static int lj92_huffman(struct lj92_stream *s, int len)
{
    while (len >= 17)
    {
        int id = lj92_byte(s);
        if (id < 0 || id > 3) return 0;
        struct lj92_table *t = &s->tables[id];
        int total = 0, code = 0;
        for (int i = 0; i < 16; i++)
        {
            int n = lj92_byte(s);
            if (n < 0 || code + n > (2 << i) || total + n > 256) return 0;
            t->count[i] = n;
            t->first[i] = code;
            t->start[i] = total;
            total += n;
            code = (code + n) << 1;
        }
        if (!total || len < 17 + total) return 0;
        for (int i = 0; i < total; i++)
        {
            int symbol = lj92_byte(s);
            if (symbol < 0 || symbol > 16) return 0;
            t->symbols[i] = symbol;
        }
        memset(t->fast, 0, sizeof(t->fast));
        for (int i = 0; i < 8; i++)
        {
            int shift = 7 - i;
            for (int j = 0; j < t->count[i]; j++)
            {
                int first = (t->first[i] + j) << shift;
                for (int k = 0; k < (1 << shift); k++)
                    t->fast[first + k] = ((i + 1) << 8) | t->symbols[t->start[i] + j];
            }
        }
        t->defined = 1;
        len -= 17 + total;
    }
    return len == 0;
}

static void lj92_fill_bits(struct lj92_stream *s, int count)
{
    while (s->available < count && !s->marker)
    {
        int byte = lj92_byte(s);
        if (byte == 255)
        {
            do { s->marker = lj92_byte(s); } while (s->marker == 255);
            if (s->marker) break;
        }
        if (byte < 0) { s->marker = -1; break; }
        s->bits = (s->bits << 8) | byte;
        s->available += 8;
    }
}

static int lj92_get_bits(struct lj92_stream *s, int count)
{
    lj92_fill_bits(s, count);
    if (s->available < count) return -1;
    s->available -= count;
    return (s->bits >> s->available) & ((1u << count) - 1);
}

static int lj92_symbol(struct lj92_stream *s, const struct lj92_table *t)
{
    lj92_fill_bits(s, 8);
    if (s->available >= 8)
    {
        unsigned index = (s->bits >> (s->available - 8)) & 255;
        unsigned pair = t->fast[index];
        if (pair)
        {
            s->available -= pair >> 8;
            return pair & 255;
        }
    }
    int code = 0;
    for (int i = 0; i < 16; i++)
    {
        int bit = lj92_get_bits(s, 1);
        if (bit < 0) break;
        code = (code << 1) + bit;
        int index = code - t->first[i];
        if (index >= 0 && index < t->count[i]) return t->symbols[t->start[i] + index];
    }
    return -1;
}

static int lj92_predict(int id, int left, int above, int corner)
{
    switch (id)
    {
        case 1: return left;
        case 2: return above;
        case 3: return corner;
        case 4: return left + above - corner;
        case 5: return left + ((above - corner) >> 1);
        case 6: return above + ((left - corner) >> 1);
        default: return (left + above) >> 1;
    }
}

static int lj92_decode_rows(struct lj92_stream *s, unsigned width, unsigned height,
                           int components, const int *tables, int predictor, int precision,
                           int point_transform, int out_bpp, uint8_t *out)
{
    uint16_t *row = malloc(width * components * sizeof(*row));
    if (!row) return 0;
    uint32_t packed = 0;
    int packed_bits = 0, ok = 0;
    unsigned mask = (1u << (precision - point_transform)) - 1;
    for (unsigned y = 0; y < height && !ml_shutdown_requested; y++)
    {
        int left[4] = {0}, corner[4] = {0};
        for (unsigned x = 0; x < width; x++)
        {
            for (int c = 0; c < components; c++)
            {
                uint16_t *sample = &row[x * components + c];
                int above = y ? *sample : 0;
                int predicted = !y ? (x ? left[c] : (int)((mask + 1) / 2))
                                   : (x ? lj92_predict(predictor, left[c], above, corner[c]) : above);
                int size = lj92_symbol(s, &s->tables[tables[c]]);
                if (size < 0) goto done;
                int diff = 0;
                if (size == 16) diff = -32768;
                else if (size)
                {
                    diff = lj92_get_bits(s, size);
                    if (diff < 0) goto done;
                    if (diff < (1 << (size - 1))) diff -= (1 << size) - 1;
                }
                *sample = left[c] = (predicted + diff) & mask;
                corner[c] = above;
                packed = (packed << out_bpp) | ((unsigned)*sample << (out_bpp - precision + point_transform));
                packed_bits += out_bpp;
                if (packed_bits >= 16)
                {
                    packed_bits -= 16;
                    unsigned word = packed >> packed_bits;
                    *out++ = word;
                    *out++ = word >> 8;
                }
            }
        }
    }
    if (!ml_shutdown_requested && !packed_bits)
        ok = (s->marker ? s->marker : lj92_marker(s)) == 0xd9;
done:
    free(row);
    return ok;
}

static int lj92_decode_file(FILE *file, unsigned bytes, void *out, unsigned capacity,
                            unsigned width, unsigned height, int precision, int out_bpp)
{
    if (!out || !width || width > 16384 || !height || height > 16384 || (width & 7) ||
        precision < 8 || precision > out_bpp || (out_bpp != 14 && out_bpp != 16) ||
        capacity < (width / 8) * height * out_bpp) return 0;
    struct lj92_stream *s = fio_malloc(sizeof(*s));
    if (!s) return 0;
    memset(s, 0, sizeof(*s));
    s->file = file;
    s->remaining = bytes;
    unsigned jpeg_w = 0, jpeg_h = 0;
    int ids[4], components = 0, ok = 0;
    if (lj92_marker(s) != 0xd8) goto done;
    for (int marker; (marker = lj92_marker(s)) > 0 && marker != 0xd9;)
    {
        int len = lj92_word(s) - 2;
        if (len < 0) break;
        if (marker == 0xc3)
        {
            if (components || len < 6 || lj92_byte(s) != precision) break;
            jpeg_h = lj92_word(s);
            jpeg_w = lj92_word(s);
            components = lj92_byte(s);
            if (!jpeg_w || jpeg_w > 16384 || !jpeg_h || jpeg_h > 16384 || components < 1 ||
                components > 4 || len != 6 + 3 * components ||
                (uint64_t)jpeg_w * jpeg_h * components != width * height) break;
            for (int c = 0; c < components; c++)
            {
                ids[c] = lj92_byte(s);
                if (ids[c] < 0 || lj92_byte(s) != 0x11 || lj92_byte(s) != 0) goto done;
                for (int previous = 0; previous < c; previous++)
                    if (ids[c] == ids[previous]) goto done;
            }
        }
        else if (marker == 0xc4)
        {
            if (!lj92_huffman(s, len)) break;
        }
        else if (marker == 0xda)
        {
            if (!components || len != 4 + 2 * components || lj92_byte(s) != components) break;
            int tables[4];
            for (int c = 0; c < components; c++)
            {
                if (lj92_byte(s) != ids[c]) goto done;
                int selector = lj92_byte(s);
                if (selector < 0 || selector > 0x30 || (selector & 15) ||
                    !s->tables[selector >> 4].defined) goto done;
                tables[c] = selector >> 4;
            }
            int predictor = lj92_byte(s);
            int end = lj92_byte(s);
            int transform = lj92_byte(s);
            if (predictor < 1 || predictor > 7 || end != 0 || transform < 0 || transform >= precision) break;
            ok = lj92_decode_rows(s, jpeg_w, jpeg_h, components, tables, predictor,
                                  precision, transform, out_bpp, out);
            break;
        }
        else if (marker == 0xdd)
        {
            if (len != 2 || lj92_word(s) != 0) break;
        }
        else
        {
            while (len--) if (lj92_byte(s) < 0) goto done;
        }
    }
done:
    fio_free(s);
    return ok;
}

#endif
