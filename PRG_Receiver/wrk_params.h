int get_params_num(void);
char* get_params_name(unsigned char indx);
void Param_to_str(unsigned char *buf,int indx);
void Str_to_param(unsigned char *buf,int indx);
void Restore_default_settings(void);
void Save_Params_To_EEPROM(void);
unsigned char Restore_settings_from_eeprom(void);
