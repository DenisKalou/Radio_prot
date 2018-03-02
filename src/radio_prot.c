/*
 ============================================================================
 Name        : protRadio.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "libs/radio_prot_api.h"
#include "libs/radio_prot_h.h"

uint8_t buffer[1024];

void test_seek(uint8_t* data, protRadio *rad);
void test_seek_ext(uint8_t* data, protRadio *rad);
void test_content(uint8_t* data, protRadio *rad, uint8_t frag);
void test_file(uint8_t *data, protRadio *rad, uint8_t frag);
void test_hf_content(uint8_t *data, protRadio *rad);
void test_filter(uint8_t *data, protRadio *rad, uint8_t frag);
void test_set_content(uint8_t *data, protRadio *rad, uint8_t frag);
void test_set_ts(uint8_t *data, protRadio *rad);
void test_set_prop(uint8_t *data, protRadio *rad, uint8_t qp);
void test_get_prop(uint8_t *data, protRadio *rad, uint8_t qp);
void test_set_file(uint8_t *data, protRadio *rad, uint8_t frag);
void test_get_crc_file(uint8_t *data, protRadio *rad);
void test_get_crc_tc(uint8_t *data, protRadio *rad);
void test_set_str(uint8_t* data, protRadio *rad);
void test_get_str(uint8_t* data, protRadio *rad);
void test_get_state(uint8_t* data, protRadio *rad);
void test_set_date_time(uint8_t *data, protRadio *rad);

void radio_send(uint8_t* data, size_t len) {
	printf("\n******Radio sent \nlen = %d\n", len);
	printf("----------------pkt begin\n");
	for (int i = 0; i < len; ++i) {
		printf("data[%d] = %d\n", i, data[i]);
	}
	printf("----------------pkt end\n");
	if (data[8] == 35){
		fr_get_bstr *a = (fr_get_bstr*)data;
		printf("got string: ");
		for (int i = 0; i < a->lens; ++i){
			printf("%c", a->str[i]);
		}
		printf("\n");
	}
}

int main(void) {
	set_data();
	uint8_t data[256];

	protRadio *rad = prot_radio;
	rad->radio_send = &radio_send;
	prot_init(rad);
	//rad->send_status(0);
	//test_seek_ext(data, rad);
	test_set_date_time(data, rad);
	//test_get_state(data, rad);
	//test_file(data, rad, 2);
	return 0;
}

void test_seek(uint8_t* data, protRadio *rad) {
	fa_seek *s = (fa_seek*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->crc2 = 0;
	s->lp = 0;
	s->crc = 0xFFFF;
	s->capfl = 0;
	s->dev.func = FASEEK;
	printf("SEEK LENGTH = %d\n", sizeof(fr_seek));
	rad->radio_recv(data, sizeof(fa_seek));
}

void test_seek_ext(uint8_t* data, protRadio *rad) {
	fa_seek *s = (fa_seek*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->crc2 = 0xFFFF;
	s->lp = 0;
	s->crc = 0xFFFF;
	s->capfl = 290;
	s->dev.func = FASEEK;
	printf("SEEK_EXT LENGTH = %d\n", sizeof(fr_seek_ext));
	rad->radio_recv(data, sizeof(fa_seek));
}
void test_content(uint8_t* data, protRadio *rad, uint8_t frag) {
	fa_content *s = (fa_content*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->clpl = 0b01001100;
	s->nftk = frag;
	s->crc = 0xFFFF;
	s->dev.func = FACONTENT;
	printf("CONTENT LENGTH = %d\n", sizeof(fr_content));
	rad->radio_recv(data, sizeof(fa_content));
}
void test_hf_content(uint8_t *data, protRadio *rad) {
	fa_hf_content *s = (fa_hf_content*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->clpl = 0b01001100;
	s->crc = 0xFFFF;
	s->dev.func = FAHFCONTENT;
	printf("HF CONT LENGTH = %d\n", sizeof(fa_hf_content));
	rad->radio_recv(data, sizeof(fa_hf_content));
}
void test_file(uint8_t *data, protRadio *rad, uint8_t frag) {
	fa_file *s = (fa_file*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->nf = 1;
	s->nff = frag;
	s->crc = 0xFFFF;
	s->dev.func = FAFILE;
	printf("FILE LENGTH = %d\n", sizeof(fr_file));
	rad->radio_recv(data, sizeof(fr_file));
}
void test_filter(uint8_t *data, protRadio *rad, uint8_t frag) {
	fa_call_filter *s = (fa_call_filter*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->l = 1;
	s->nFiltra = frag;
	s->crc = 0xFFFF;
	s->np = 4;
	s->dev.func = FACALLFILTER;
	printf("FILTER LENGTH = %d\n", sizeof(fa_call_filter));
	rad->radio_recv(data, sizeof(fa_call_filter));
}
void test_set_ts(uint8_t *data, protRadio *rad) {
	fa_set_ts *s = (fa_set_ts*)data;
	s->dev.dev_id_from = 8;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->tts = 255;
	s->nts = 0xB3B2;
	s->lts = 121;
	s->dir = 2;
	s->crc = 0xFFFF;
	s->dev.func = FASETTS;
	printf("SET TS LENGTH = %d\n", sizeof(fa_set_ts));
	rad->radio_recv(data, sizeof(fa_set_ts));
}
void test_set_prop(uint8_t *data, protRadio *rad, uint8_t qp) {
	fa_set_prop *s = (fa_set_prop*)data;
	s->dev.dev_id_from = 8;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->nfp = 2;
	s->qp = qp;
	size_t size = sizeof(fa_set_prop) + s->qp * 2 + 2;
	for (int i = 0; i < s->qp; ++i) {
		s->vp[i] = 0xFF00 + i;
	}
	while(size % 8) data[size++ - 2] = 0;
	*(uint16_t*)(data + size - 2) = 0xFFFF;
	s->dev.func = FASETPROP;
	printf("SET PROP LENGTH = %d\n", size);
	rad->radio_recv(data, size);
}
void test_get_prop(uint8_t *data, protRadio *rad, uint8_t qp) {
	fa_get_prop *s = (fa_get_prop*)data;
	s->dev.dev_id_from = 8;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->nfp = 2;
	s->qp = qp;
	s->crc = 0xFFFF;
	s->dev.func = FAGETPROP;
	printf("GET PROP LENGTH = %d\n", sizeof(fa_get_prop));
	rad->radio_recv(data, sizeof(fa_get_prop));
}
void test_set_content(uint8_t *data, protRadio *rad, uint8_t frag) {
	fa_set_content *s = (fa_set_content*)data;
	s->dev.dev_id_from = 8;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->clpl = 5;
	s->lenFr = 128;
	s->nftk = frag;
	for (int i = 0; i < s->lenFr; ++i) {
		s->data[i] = i;
	}
	*(uint16_t*)(s->data + s->lenFr) = 0xFFFF;
	s->dev.func = FASETCONTENT;
	printf("SET CONT LENGTH = %d\n", sizeof(fa_set_content) + s->lenFr + 2);
	rad->radio_recv(data, sizeof(fa_set_content) + s->lenFr + 2);
}
void test_set_file(uint8_t *data, protRadio *rad, uint8_t frag) {
	fa_set_file *s = (fa_set_file*)data;
	s->dev.dev_id_from = 8;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->nf = 0x0201;
	s->lenFr = 56;
	s->nff = frag;
	for (int i = 0; i < s->lenFr; ++i) {
		s->data[i] = i;
	}
	*(uint16_t*)(s->data + s->lenFr) = 0xFFFF;
	s->dev.func = FASETFILE;
	printf("SET FILE LENGTH = %d\n", sizeof(fa_set_file) + s->lenFr + 2);
	rad->radio_recv(data, sizeof(fa_set_file) + s->lenFr + 2);
}
void test_get_crc_file(uint8_t *data, protRadio *rad) {
	fa_get_crc_file *s = (fa_get_crc_file*)data;
	s->dev.dev_id_from = 8;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->nf = 5;
	s->crc = 0xFFFF;
	s->dev.func = FAGETCRCFILE;
	printf("GET CRC FILE LENGTH = %d\n", sizeof(fa_get_crc_tc));
	rad->radio_recv(data, sizeof(fa_get_crc_tc));
}
void test_get_crc_tc(uint8_t *data, protRadio *rad) {
	fa_get_crc_tc *s = (fa_get_crc_tc*)data;
	s->dev.dev_id_from = 8;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->ktu = 0b01001100;
	s->crc = 0xFFFF;
	s->dev.func = FAGETCRCTC;
	printf("GET CRC TC LENGTH = %d\n", sizeof(fa_get_crc_tc));
	rad->radio_recv(data, sizeof(fa_get_crc_tc));
}
void test_set_str(uint8_t* data, protRadio *rad) {
	fa_set_bstr *s = (fa_set_bstr*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->lens = 11;
	uint8_t str1[11] = "Set string!";
	for(int i = 0; i < s->lens; ++i){
		s->str[i] = str1[i];
	}
	*(uint16_t*)(s->str + s->lens) = 0xFFFF;
	s->dev.func = FASETBSTR;
	printf("SET STR LENGTH = %d\n", sizeof(fa_set_bstr) + 11 + 2);
	rad->radio_recv(data, sizeof(fa_set_bstr) + 11 + 2);
}
void test_get_str(uint8_t* data, protRadio *rad) {
	fa_get_bstr *s = (fa_get_bstr*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->nums = 3;
	s->crc = 0xFFFF;
	s->dev.func = FAGETBSTR;
	printf("GET STR LENGTH = %d\n", sizeof(fa_get_bstr));
	rad->radio_recv(data, sizeof(fa_get_bstr));
}
void test_set_date_time(uint8_t *data, protRadio *rad) {
	fa_set_date_time *s = (fa_set_date_time*)data;
	s->dev.dev_id_from = 8;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->mon = 5;
	s->day = 5;
	s->hour = 12;
	s->min = 35;
	s->sec = 43;
	s->crc = 0xFFFF;
	s->dev.func = FASETDATETIME;
	printf("SET DATE TIME LENGTH = %d\n", sizeof(fa_set_date_time));
	rad->radio_recv(data, sizeof(fa_set_date_time));
}
void test_get_state(uint8_t* data, protRadio *rad) {
	fa_get_state *s = (fa_get_state*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->door_open = 0;
	s->crc = 0xFFFF;
	s->dev.func = FAGETSTATE;
	printf("GET STATE LENGTH = %d\n", sizeof(fa_get_state));
	rad->radio_recv(data, sizeof(fa_get_state));
}
