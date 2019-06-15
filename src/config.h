#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILE "PLATO.SET"

#include <stdbool.h>

typedef struct _configInfo {
  int baud;
  char init_str[64];
  char dial_str[64];
} ConfigInfo;

/**
 * Load config if exists, give defaults if not.
 */
void config_init(void);

/**
 * Load configuration
 */
bool config_load(void);

/**
 * Save configuration
 */
void config_save(void);

/**
 * config set defaults
 */
void config_set_defaults(void);

/**
 * Set desired baud rate
 */
void config_baud_set(int new_baud);

/**
 * Set old baud rate (CANCEL)
 */
void config_baud_set_old(void);

/**
 * Set new baud rate
 */
void config_baud_set_new(void);

#endif /* CONFIG_H */
