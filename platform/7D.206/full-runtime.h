#ifndef FULL_RUNTIME_7D206_H
#define FULL_RUNTIME_7D206_H
int full_property_valid(unsigned property, const void *buf, unsigned len);
int full_property_write_allowed(unsigned property);
int full_property_write_valid(unsigned property, const void *buf, unsigned len);
/* Set when a startup contract fails; ML tasks are then not started. */
extern volatile int full_failure;
#endif
