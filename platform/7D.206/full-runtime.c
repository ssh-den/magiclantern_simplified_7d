/* 7D.206 property guards: reject malformed property buffers and limit
 * property writes to reviewed IDs and payload sizes. */
#include "dryos.h"
#include "property.h"
#include "full-runtime.h"

struct property_bound { uint32_t id; uint16_t min, max; };
static const struct property_bound bounds[] = {
#define PORT_PROPERTY(id,lo,hi) {id,lo,hi},
#include "property-contracts.h"
#undef PORT_PROPERTY
};
struct property_write { uint32_t id; unsigned len; };
static const struct property_write writes[] = {
#define PORT_WRITE(id,len) {id,len},
#include "property-writes.h"
#undef PORT_WRITE
};

volatile int full_failure;

int full_property_write_allowed(unsigned property)
{
    for (unsigned i=0;i<COUNT(writes);i++) if (writes[i].id==property) return 1;
    return 0;
}

static int full_counter_value_valid(unsigned property, const void *buf)
{
    if (property != PROP_FOLDER_NUMBER_A && property != PROP_FILE_NUMBER_A &&
        property != PROP_NUMBER_OF_CONTINUOUS_MODE) return 1;

    uint32_t value;
    memcpy(&value, buf, sizeof(value));
    if (property == PROP_FOLDER_NUMBER_A) return value >= 100 && value <= 999;
    return value <= 9999;
}

int full_property_write_valid(unsigned property, const void *buf, unsigned len)
{
    for (unsigned i=0;i<COUNT(writes);i++) {
        if (writes[i].id!=property) continue;
        return buf && len==writes[i].len && full_counter_value_valid(property, buf);
    }
    return 0;
}

int full_property_valid(unsigned property, const void *buf, unsigned len)
{
    for (unsigned i=0; i<COUNT(bounds); i++) {
        if (bounds[i].id != property) continue;
        return (buf || !len) && len >= bounds[i].min &&
               (!bounds[i].max || len <= bounds[i].max);
    }
    /* Third-party modules own their callback ABI. */
    return buf || !len;
}
