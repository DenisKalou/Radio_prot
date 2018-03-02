#include "radio_prot_h.h"
#define W_LEN 120
#define F_LEN 128 /*максимальная длина фрагмента*/
#if PROT_MASTER

#else
extern uint8_t buffer[1024];
void set_data(){
	for (int i = 0; i < 256; ++i){
		buffer[i] = (uint8_t)i;
	}
}
void get_file(uint8_t file_num, size_t* len, uint8_t frag, uint8_t* data){
	*len = 256;
	uint8_t max_fr = *len / W_LEN;
	if (*len % W_LEN) max_fr++;
	if (frag + 1 < max_fr){
		for (int i = W_LEN * frag; i < W_LEN * (frag + 1) ; ++i){
			data[i - W_LEN * frag] = buffer[i];
			printf("buff[%i] = %d\n", i, buffer[i]);
		}
	} else
	{
		for (int i = W_LEN * frag; i < W_LEN * frag + (*len % W_LEN) ; ++i){
			data[i - W_LEN * frag] = buffer[i];
			printf("buff[%i] = %d\n", i, buffer[i]);
		}
	}
}
void set_file(uint8_t file_num, uint8_t frag, size_t len, uint8_t* data){
	for(int i = 0; i < len; ++i){
		printf("data[%d] = %d\n", i, data[i]);
	}
}
void get_crc_file(uint8_t file_num, uint16_t* crcf) {
	*crcf = 0xABCD;
}
void get_content(uint8_t clpl, size_t* len, uint8_t frag, uint8_t* data){
	*len = 136;
	uint8_t max_len = *len / F_LEN;
	if (*len % F_LEN) max_len++;
	if (frag + 1 < max_len){
		for (int i = F_LEN * frag; i < F_LEN * (frag + 1) ; ++i){
			data[i - F_LEN * frag] = buffer[i];
		}
	} else
	{
		for (int i = F_LEN * frag; i < F_LEN * frag + (*len % F_LEN) ; ++i){
			data[i - F_LEN * frag] = buffer[i];
			printf("buff[%i] = %d\n", i, buffer[i]);
		}
	}
}
void get_hf_content(uint8_t clpl, uint16_t* hf){
	*hf = 0xABCD;
}
void set_content(uint8_t clpl, uint8_t frag, size_t len, uint8_t* data){
	for(int i = 0; i < len; ++i){
		printf("data[%d] = %d\n", i, data[i]);
	}
}
uint8_t call_filter(uint8_t lang, uint8_t filter_point_num, size_t* len, uint8_t frag, uint8_t* data){
	if (filter_point_num == 5){
		*len = 1;
		data[0] = 0;
		printf("EMERGENCY!!!!");
		return 0;
	}
	*len = 130;
	uint8_t max_len = *len / F_LEN;
	if (*len % F_LEN) max_len++;
	if (frag + 1 < max_len){
		for (int i = F_LEN * frag; i < F_LEN * (frag + 1) ; ++i){
			data[i - F_LEN * frag] = buffer[i];
		}
	} else
	{
		for (int i = F_LEN * frag; i < F_LEN * frag + (*len % F_LEN) ; ++i){
			data[i - F_LEN * frag] = buffer[i];
			printf("buff[%i] = %d\n", i, buffer[i]);
		}
	}
	return 0;
}
void set_filter(uint8_t lang, uint8_t filter_point_num, uint8_t max_frag, uint8_t frag, uint8_t* data){
	return;
}
void get_ts(uint8_t* t_type, uint16_t* t_num, uint8_t* t_lit, uint8_t* t_dir){
	*t_type = 191;
	*t_num = 0xC1C0;
	*t_lit = 194;
	*t_dir = 195;
	printf("GET_TS\ntype = %d\nnum = %d\nletera = %d\ndirection = %d\n", *t_type, *t_num, *t_lit, *t_dir);
}
void set_ts(uint8_t t_type, uint8_t t_num, uint8_t t_lit, uint8_t t_dir){
	printf("SET_TS\ntype = %d\nnum = %d\nletera = %d\ndirection = %d\n", t_type, t_num, t_lit, t_dir);
}
void get_params(uint8_t param_num, uint8_t params_qty, uint16_t *params_data){
	for (int i = 0; i < params_qty; ++i){
		params_data[i] = 0xAA00 + i;
	}
}
uint8_t set_params(uint8_t param_num, uint8_t params_qty, uint16_t *params_data){
	printf("SETTING PARAMS\n");
	for (int i = 0; i < params_qty; ++i){
		printf("params_data[%d] = %d\n", i, params_data[i]);
	}
	return 0;
}
uint16_t crc16_calc(uint8_t* data, size_t len){
	return 0xFFFF;
}
void get_device_data(uint8_t *id, uint32_t *sn){
	*id = 64;
	*sn = 0xAAAAAA;
}
void get_params_data(uint8_t al[5], uint8_t* lp, uint16_t* ap, uint8_t* h,
					uint8_t* fl){
	uint8_t step = 5;
	while(step--) al[step] = 55 - step;
	*lp = 0;
	*ap = 300;
	*h = 20;
	*fl = 7;
}
uint8_t set_str(uint8_t str_num, size_t str_len, uint8_t* data){
	printf("set string: \"");
	for(int i = 0; i < str_len; ++i){
		printf("%c", data[i]);
	}
	printf("\"\n");
	return 0;
}
void get_str(uint8_t str_num, size_t* str_len, uint8_t data[]){
	uint8_t str[] = "Hello World";
	*str_len = sizeof(str) - 1;
	for (int i = 0; i < *str_len; ++i){
		data[i] = str[i];
	}
}
void get_date_time(uint8_t* mon, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec){
	*mon = 6;
	*day = 1;
	*hour = 1;
	*min = 15;
	*sec = 55;
	printf("GET_DATE_TIME\nmon = %d\nday = %d\nhour = %d\nmin = %d\nsec = %d\n",
				*mon, *day, *hour, *min, *sec);
}
void set_date_time(uint8_t mon, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec){
	printf("SET_DATE_TIME\nmon = %d\nday = %d\nhour = %d\nmin = %d\nsec = %d\n",
			mon, day, hour, min, sec);
}
void prot_init(protRadio *rad){
	//rad = prot_radio;
	rad->hwRadioAAL.set_date_time = &set_date_time;
	rad->hwRadioAAL.get_date_time = &get_date_time;
	rad->hwRadioAAL.get_str = &get_str;
	rad->hwRadioAAL.set_str = &set_str;
	rad->hwRadioAAL.crc16_calc = &crc16_calc;
	rad->hwRadioAAL.get_content = &get_content;
	rad->hwRadioAAL.get_crc_file = &get_crc_file;
	rad->hwRadioAAL.get_device_data = &get_device_data;
	rad->hwRadioAAL.get_file = &get_file;
	rad->hwRadioAAL.call_filter = &call_filter;
	rad->hwRadioAAL.get_hf_content = &get_hf_content;
	rad->hwRadioAAL.get_params = &get_params;
	rad->hwRadioAAL.get_params_data = &get_params_data;
	rad->hwRadioAAL.get_ts = &get_ts;
	rad->hwRadioAAL.set_content = &set_content;
	rad->hwRadioAAL.set_file = &set_file;
	rad->hwRadioAAL.set_filter = &set_filter;
	rad->hwRadioAAL.set_params = &set_params;
	rad->hwRadioAAL.set_ts = &set_ts;
}
/*void radio_send(uint8_t* data, size_t len){
	return;
}*/
#endif
