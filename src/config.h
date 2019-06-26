#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILE "PLATO.SET"

#include <stdbool.h>

typedef struct _configInfo {
  unsigned char baud;
  unsigned char serial_port;
  char init_str[64];
  unsigned char entry_selected;
  char entry1_name[32];
  char entry2_name[32];
  char entry3_name[32];
  char entry4_name[32];
  char entry1_dial[32];
  char entry2_dial[32];
  char entry3_dial[32];
  char entry4_dial[32];
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
