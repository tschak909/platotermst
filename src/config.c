#include "config.h"

ConfigInfo config;

static int baud_rate;

/**
 * Load config if exists, give defaults if not.
 */
void config_init(void)
{
  /* memset(&config,0,sizeof(ConfigInfo)); */
  
  /* if (config_load()==false) */
  /*   { */
  /*     config_set_defaults(); */
  /*     config_save(); */
  /*   } */
}

/**
 * Load configuration
 */
bool config_load(void)
{
  /* FILE *fp; */
  /* fp=fopen(CONFIG_FILE,"r"); */

  /* if (fp==NULL) */
  /*   return false; */

  /* fread(&config,1,sizeof(ConfigInfo),fp); */
  /* fclose(fp); */
  /* return true; */
}

/**
 * Save configuration
 */
void config_save(void)
{
  /* FILE *fp; */
  /* fp=fopen(CONFIG_FILE,"w"); */
  /* if (!fp) */
  /*   return; */

  /* fwrite(&config,1,sizeof(ConfigInfo),fp); */
  /* fclose(fp); */
}

/**
 * config set defaults
 */
void config_set_defaults(void)
{
  /* config.baud=7; */
  /* strcpy(config.init_str,"ATZ"); */
  /* strcpy(config.dial_str,"ATDTIRATA.ONLINE:8005"); */
}

/**
 * Set desired baud rate
 */
void config_baud_set(int new_baud)
{
  /* baud_rate=new_baud; */
}

/**
 * Set old baud rate (CANCEL)
 */
void config_baud_set_old(void)
{
  /* baud_rate=config.baud; */
}

/**
 * Set new baud rate
 */
void config_baud_set_new(void)
{
  /* config.baud=baud_rate; */
}
