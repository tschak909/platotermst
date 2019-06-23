#include "config.h"
#include <string.h>
#include <gem.h>
#include <osbind.h>

ConfigInfo config;

static int baud_rate;

/**
 * Load config if exists, give defaults if not.
 */
void config_init(void)
{
  memset(&config,0,sizeof(ConfigInfo));
  if (config_load()==false)
    {
      config_set_defaults();
      config_save();
    }
}

/**
 * Load configuration
 */
bool config_load(void)
{
  int fp;

  fp=Fopen(CONFIG_FILE,FO_READ);
  if (fp>0)
    {
      Fread(fp,sizeof(ConfigInfo),&config);
    }
  else
    {
      return false;
    }
  
  Fclose(fp);
  return true;
}

/**
 * Save configuration
 */
void config_save(void)
{
  int fp;
  
  fp=Fcreate(CONFIG_FILE,0);
  if (fp>0)
    {
      Fwrite(fp,sizeof(ConfigInfo),&config);
    }
  else
    return;
  
  Fclose(fp);
}

/**
 * config set defaults
 */
void config_set_defaults(void)
{
  config.baud=7;
  strcpy(config.init_str,"ATZ");
  strcpy(config.entry1_name,"IRATA.ONLINE");
  strcpy(config.entry2_name,"CYBER1.ORG");
  strcpy(config.entry3_name,"EMPTY");
  strcpy(config.entry4_name,"EMPTY");
  strcpy(config.entry1_dial,"ATDTIRATA.ONLINE:8005");
  strcpy(config.entry2_dial,"ATDTCYBERSERV.ORG:8005");
  strcpy(config.entry3_dial,"");
  strcpy(config.entry4_dial,"");
  config.entry_selected=5;
}
