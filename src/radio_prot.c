/*
 ============================================================================
 Name        : radio_prot.c
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

void test_seek(uint8_t* data, radio_prot *rad);
void test_seek_ext(uint8_t* data, radio_prot *rad);
void test_content(uint8_t* data, radio_prot *rad, uint8_t frag);
void test_file(uint8_t *data, radio_prot *rad, uint8_t frag);
void test_hf_content(uint8_t *data, radio_prot *rad);
void test_filter(uint8_t *data, radio_prot *rad, uint8_t frag);


void radio_send(uint8_t* data, size_t len){
	printf("len = %I64u\n", len);
	printf("----------------pkt begin\n");
	for (int i = 0; i < len; ++i){
		printf("data[%d] = %d\n", i, data[i]);
	}
	printf("----------------pkt end\n");
}

int main(void) {
	set_file(0, 0, 0, 0);
	uint8_t data[256];

	radio_prot rad;
	rad.al[0] = 50;
	rad.al[1] = 51;
	rad.al[2] = 52;
	rad.al[3] = 53;
	rad.al[4] = 54;
	rad.ap = 300;
	rad.dev_id = 2;
	rad.dev_sn = 0xAAAAAA;
	rad.door_open = 1;
	rad.fl = 7;
	rad.h = 20;
	rad.lp = 0;
	prot_init(&rad);
	test_filter(data, &rad, 0);

	return EXIT_SUCCESS;
}

void test_seek(uint8_t* data, radio_prot *rad){
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
	printf("SEEK LENGTH = %I64u\n", sizeof(fr_seek));
	rad->radio_recv(data, sizeof(fa_seek));
}

void test_seek_ext(uint8_t* data, radio_prot *rad){
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
	printf("SEEK_EXT LENGTH = %I64u\n", sizeof(fr_seek_ext));
	rad->radio_recv(data, sizeof(fa_seek));
}

void test_content(uint8_t* data, radio_prot *rad, uint8_t frag){
	fa_content *s = (fa_content*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->clpl = 0b01001100;
	s->nftk = frag;
	s->crc = 0xFFFF;
	s->dev.func = FACONTENT;
	printf("CONTENT LENGTH = %I64u\n", sizeof(fr_content));
	rad->radio_recv(data, sizeof(fa_content));
}
void test_hf_content(uint8_t *data, radio_prot *rad){
	fa_hf_content *s = (fa_hf_content*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->clpl = 0b01001100;
	s->crc = 0xFFFF;
	s->dev.func = FAHFCONTENT;
	printf("FILE LENGTH = %I64u\n", sizeof(fa_hf_content));
	rad->radio_recv(data, sizeof(fa_hf_content));
}
void test_file(uint8_t *data, radio_prot *rad, uint8_t frag){
	fa_file *s = (fa_file*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->nf = 1;
	s->nff = frag;
	s->crc = 0xFFFF;
	s->dev.func = FAFILE;
	printf("FILE LENGTH = %I64u\n", sizeof(fr_file));
	rad->radio_recv(data, sizeof(fr_file));
}
void test_filter(uint8_t *data, radio_prot *rad, uint8_t frag){
	fa_call_filter *s = (fa_call_filter*)data;
	s->dev.dev_id_from = 1;
	s->dev.dev_id_to = 2;
	s->dev.dev_sn_from = 0xFFFFFF;
	s->dev.dev_sn_to = 0xAAAAAA;
	s->l = 1;
	s->nFiltra = frag;
	s->crc = 0xFFFF;
	s->np = 21;
	s->dev.func = FACALLFILTER;
	printf("FILE LENGTH = %I64u\n", sizeof(fa_call_filter));
	rad->radio_recv(data, sizeof(fa_call_filter));
}