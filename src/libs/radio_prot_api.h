#define W_LEN 120
#define F_LEN 128 /*максимальная длина фрагмента*/
#if PROT_MASTER

#else
extern uint8_t buffer[1024];
void get_file(uint8_t file_num, size_t* len, uint8_t frag, uint8_t* data){
	*len = 420;
	uint8_t max_fr = *len / W_LEN;
	if (*len % W_LEN) max_fr++;
	if (frag + 1 < max_fr){
		for (int i = W_LEN * frag; i < W_LEN * (frag + 1) ; ++i){
			data[i - W_LEN * frag] = buffer[i];
			printf("buff[%i] = %d\n", i, buffer[i]);
		}
	} else
	{
		for (int i = W_LEN * frag; i < *len % W_LEN ; ++i){
			data[i - W_LEN * frag] = buffer[i];
		}
	}
}
void set_file(uint8_t file_num, uint8_t frag, size_t len, uint8_t* data){
	for (int i = 0; i < 1024; ++i){
		buffer[i] = (uint8_t)i;
	}
}
void get_content(uint8_t clpl, size_t* len, uint8_t frag, uint8_t* data){
	*len = 625;
	uint8_t max_len = *len / F_LEN;
	if (*len % F_LEN) max_len++;
	if (frag + 1 < max_len){
		for (int i = F_LEN * frag; i < F_LEN * (frag + 1) ; ++i){
			data[i - F_LEN * frag] = buffer[i];
		}
	} else
	{
		for (int i = F_LEN * frag; i < *len % F_LEN ; ++i){
			data[i - F_LEN * frag] = buffer[i];
		}
	}
}
void get_hf_content(uint8_t clpl, uint16_t* hf){
	*hf = 0xABCD;
}
void set_content(uint8_t clpl, uint8_t frag, uint8_t len, uint8_t* data){
	return;
}
void get_filter(uint8_t lang, uint8_t filter_point_num, size_t* len, uint8_t frag, uint8_t* data){
	*len = 122;
	uint8_t max_len = *len / F_LEN;
	if (*len % F_LEN) max_len++;
	if (frag + 1 < max_len){
		for (int i = F_LEN * frag; i < F_LEN * (frag + 1) ; ++i){
			data[i - F_LEN * frag] = buffer[i];
		}
	} else
	{
		for (int i = F_LEN * frag; i < *len % F_LEN ; ++i){
			data[i - F_LEN * frag] = buffer[i];
		}
	}
}
void set_filter(uint8_t lang, uint8_t filter_point_num, uint8_t max_frag, uint8_t frag, uint8_t* data){
	return;
}
void get_ts(uint8_t* t_type, uint16_t* t_num, uint8_t* t_lit, uint8_t* t_dir){
	return;
}
void set_ts(uint8_t t_type, uint8_t t_num, uint8_t t_lit, uint8_t t_dir){
	return;
}
void get_params(uint8_t param_num, uint8_t params_qty, uint16_t *params_data){
	for (int i = 0; i < params_qty; ++i){
		params_data[i] = 0xAA00 + i;
	}
}
uint8_t set_params(uint8_t param_num, uint8_t params_qty, uint16_t *params_data){
	return 0;
}
/*void radio_send(uint8_t* data, size_t len){
	return;
}*/
#endif
