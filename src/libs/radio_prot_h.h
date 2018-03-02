#ifndef RADIO_PROT_H
#define RADIO_PROT_H

#include <stddef.h>
#include <stdint.h>

#define DEBUG 1
#define WAV_LEN 120
#define FRAG_LEN 128 /*максимальная длина фрагмента*/
#define PROT_MASTER 0 /* 0 - slave, 1 - master*/
#define FFW 2
#define ER_FFW 3
#define ER_CHECK 1
#define LF 0

#define FASEEK			0x00
#define FRSEEK			0x01
#define FACONTENT		0x02
#define FRCONTENT		0x03
#define FAHFCONTENT		0x04
#define FRHFCONTENT		0x05
#define FAFILE			0x06
#define FRFILE			0x07
#define FACALLFILTER	0x08
#define FRCALLFILTER	0x09
#define FRSEEKEXT		0x0B
#define FASETTS			0x0C
#define FRSETTS			0x0D
#define FASETPROP		0x0E
#define FRSETPROP		0x0F
#define FAGETPROP		0x10
#define FRGETPROP		0x11
#define FASETCONTENT	0x12
#define FRSETCONTENT	0x13
#define FASAVEFIRMWARE	0x14
#define FRSAVEFIRMWARE	0x15
#define FASETFILE		0x16
#define FRSETFILE		0x17
#define FAGETCRCFILE	0x18
#define FRGETCRCFILE	0x19
#define FAGETCRCTC		0x20
#define FRGETCRCTC		0x21
#define FASETBSTR		0x22
#define FRSETBSTR		0x23
#define FAGETBSTR		0x24
#define FRGETBSTR		0x25
#define FASETDATETIME	0x26
#define FRSETDATETIME	0x27
#define FAGETSTATE		0x28
#define FRGETSTATE		0x29

extern void radio_send(uint8_t* data, size_t len);

void radio_recv(uint8_t* data, size_t len);

#pragma pack(push, 1)

#if PROT_MASTER
#define MAX_DEV 4 /*максимальное количество активных радиоинформаторов*/
	typedef struct 
	{
		uint8_t dev_id;
		uint32_t dev_sn:24;
		uint8_t al[5];
		uint8_t lp;
		uint16_t ap;
		uint8_t h;
		uint8_t fl;
		uint8_t door_open;
	} input_dev;

	typedef struct
	{
		uint8_t dev_id;
		uint32_t dev_sn:24;
		uint16_t cap;
		uint8_t fl;
		input_dev *in_dev;
		void (*radio_recv)(uint8_t*, size_t);
		void (*radio_seek)();
		void (*get_file)(uint8_t, uint8_t, uint8_t*,uint8_t*);
		void (*set_file)(uint8_t, uint8_t, uint8_t*,uint8_t*);
	} radio_prot;
	

#else
	typedef struct strProtRadio
	{
		struct strHwRadioAAL
		{
			uint16_t (*crc16_calc)(uint8_t* data, size_t len);
			void (*get_device_data)(uint8_t* dev_id, uint32_t* dev_sn);
			void (*get_params_data)(uint8_t al[5], uint8_t* lp, uint16_t* ap, uint8_t* h,
					uint8_t* fl);
			void (*firmware_check)(uint16_t* version, uint16_t* last_fr);
			void (*get_file)(uint8_t file_num, size_t* len, uint8_t frag, uint8_t* data);
			void (*set_file)(uint8_t file_num, uint8_t frag, size_t len, uint8_t* data);
			void (*get_crc_file)(uint8_t file_num, uint16_t* crcf);
			void (*get_content)(uint8_t clpl, size_t* len, uint8_t frag, uint8_t* data);
			void (*get_hf_content)(uint8_t clpl, uint16_t* hf);
			void (*set_content)(uint8_t clpl, uint8_t frag, size_t len, uint8_t* data);
			uint8_t (*call_filter)(uint8_t lang, uint8_t filter_point_num, size_t* len,
					uint8_t frag, uint8_t* data);
			void (*set_filter)(uint8_t lang, uint8_t filter_point_num, uint8_t max_frag,
					uint8_t frag, uint8_t* data);
			uint8_t (*save_firmware)(uint8_t frag, uint8_t last_fr_flag, uint8_t* data);
			void (*get_ts)(uint8_t* t_type, uint16_t* t_num, uint8_t* t_lit, uint8_t* t_dir);
			void (*set_ts)(uint8_t t_type, uint8_t t_num, uint8_t t_lit, uint8_t t_dir);
			void (*get_params)(uint8_t param_num, uint8_t params_qty, uint16_t *params_data);
			uint8_t (*set_params)(uint8_t param_num, uint8_t params_qty, uint16_t *params_data);
			void (*get_str)(uint8_t str_num, size_t* str_len, uint8_t data[]);
			uint8_t (*set_str)(uint8_t str_num, size_t str_len, uint8_t* data);
			void (*get_date_time)(uint8_t* mon, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec);
			void (*set_date_time)(uint8_t mon, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
		} hwRadioAAL;
		void (*send_status) (uint8_t status);
		void (*radio_recv)(uint8_t* data, size_t len);
		void (*radio_send)(uint8_t* data, size_t len);
	} protRadio;
	extern protRadio *prot_radio;
#endif

void prot_init(protRadio*);

typedef struct
{
	uint32_t dev_id_from:8;
	uint32_t dev_sn_from:24;
	uint32_t dev_id_to:8;
	uint32_t dev_sn_to:24;
	uint8_t func;
} dev_inf;

typedef struct
{
	dev_inf dev;
	uint8_t data[];
} ptr;

/*FASEEK*/
typedef struct
{
	dev_inf dev;
	uint16_t crc;
	uint8_t lp;
	uint16_t capfl;
	uint16_t crc2;
} fa_seek;

/*FRSEEK*/
typedef struct
{
	dev_inf dev;
	uint8_t al[5];
	uint16_t crc;
} fr_seek;

/*FACONTENT*/
typedef struct
{
	dev_inf dev;
	uint32_t clpl:8;
	uint32_t nftk:8;
	uint32_t crc:16;
	uint32_t empty:24;
} fa_content;

/*FRCONTENT*/
typedef struct
{
	dev_inf dev;
	uint8_t clpl;
	uint8_t nftk;
	uint8_t aftk;
	uint16_t hf;
	uint8_t data[];
	/*uint16_t crc;*/
} fr_content;

/*FAHFCONTENT*/
typedef struct
{
	dev_inf dev;
	uint8_t clpl;
	uint16_t crc;
	uint32_t empty;
} fa_hf_content;

/*FRHFCONTENT*/
typedef struct
{
	dev_inf dev;
	uint8_t clpl;
	uint16_t hf;
	uint16_t crc;
	uint16_t empty;
} fr_hf_content;

/*FAFILE*/
typedef struct
{
	dev_inf dev;
	uint16_t nf;
	uint8_t empty0;
	uint8_t nff;
	uint16_t crc;
	uint8_t empty1;
} fa_file;

/*FRFILE*/
typedef struct
{
	dev_inf dev;
	uint16_t nf;
	uint8_t empty;
	uint8_t nff;
	uint8_t aff;
	uint8_t data[];
	/*uint16_t crc;*/
} fr_file;

/*FACALLFILTER*/
typedef struct
{
	dev_inf dev;
	uint8_t l;
	uint8_t np;
	uint8_t nFiltra;
	uint16_t crc;
	uint16_t empty;
} fa_call_filter;

/*FRCALLFILTER*/
typedef struct
{
	dev_inf dev;
	uint8_t l;
	uint8_t np;
	uint8_t nFiltra;
	uint8_t aFiltra;
	uint8_t data[];
	/*uint16_t crc;*/
} fr_call_filter;

/*FRSEEKEXT*/
typedef struct
{
	dev_inf dev;
	uint8_t al[5];
	uint8_t lp;
	uint16_t ap;
	uint8_t h;
	uint8_t fl;
	uint8_t empty[3];
	uint16_t crc;
} fr_seek_ext;

/*FASETTS*/
typedef struct
{
	dev_inf dev;
	uint8_t tts;
	uint16_t nts;
	uint8_t lts;
	uint8_t dir;
	uint16_t crc;
} fa_set_ts;

/*FRSETTS*/
typedef struct
{
	dev_inf dev;
	uint8_t tts;
	uint16_t nts;
	uint8_t lts;
	uint8_t dir;
	uint16_t crc;
} fr_set_ts;

/*FASETPROP*/
typedef struct
{
	dev_inf dev;
	uint8_t nfp;
	uint8_t qp;
	uint8_t empty;
	uint16_t vp[];
	/*uint16_t crc;*/
} fa_set_prop;

/*FRSETPROP*/
typedef struct
{
	dev_inf dev;
	uint8_t nfp;
	uint8_t qp;
	uint8_t er;
	uint16_t empty;
	uint16_t crc;
} fr_set_prop;

/*FAGETPROP*/
typedef struct
{
	dev_inf dev;
	uint8_t nfp;
	uint8_t qp;
	uint8_t empty[3];
	uint16_t crc;
} fa_get_prop;

/*FRGETPROP*/
typedef struct
{
	dev_inf dev;
	uint8_t nfp;
	uint8_t qp;
	uint8_t empty;
	uint16_t vp[];
	/*uint16_t crc;*/
} fr_get_prop;

/*FASETCONTENT*/
typedef struct
{
	dev_inf dev;
	uint8_t clpl;
	uint8_t nftk;
	uint8_t lenFr;
	uint8_t data[];
	/*uint16_t crc;*/
} fa_set_content;

/*FRSETCONTENT*/
typedef struct
{
	dev_inf dev;
	uint8_t clpl;
	uint8_t nftk;
	uint8_t empty[3];
	uint16_t crc;
} fr_set_content;

/*FASAVEFIRMWARE*/
typedef struct
{
	dev_inf dev;
	uint16_t v;
	uint16_t nf;
	uint8_t lf;
	uint8_t data[];
	/*uint16_t crc;*/
} fa_save_firmware;

/*FRSAVEFIRMWARE*/
typedef struct
{
	dev_inf dev;
	uint16_t v;
	uint16_t nf;
	uint8_t flags;
	uint16_t crc;
} fr_save_firmware;

/*FASETFILE*/
typedef struct
{
	dev_inf dev;
	uint16_t nf;
	uint8_t nff;
	uint8_t lenFr;
	uint8_t empty;
	uint8_t data[];
	/*uint16_t crc;*/
} fa_set_file;

/*FRSETFILE*/
typedef struct
{
	dev_inf dev;
	uint16_t nf;
	uint8_t nff;
	uint8_t lenFr;
	uint8_t empty;
	uint16_t crc;
} fr_set_file;

/*FAGETCRCFILE*/
typedef struct
{
	dev_inf dev;
	uint16_t nf;
	uint8_t empty[3];
	uint16_t crc;
} fa_get_crc_file;

/*FRGETCRCFILE*/
typedef struct
{
	dev_inf dev;
	uint16_t nf;
	uint16_t crcf;
	uint8_t empty;
	uint16_t crc;
} fr_get_crc_file;

/*FAGETCRCTC*/
typedef struct
{
	dev_inf dev;
	uint8_t ktu;
	uint32_t empty;
	uint16_t crc;
} fa_get_crc_tc;

/*FRGETCRCTC*/
typedef struct
{
	dev_inf dev;
	uint8_t ktu;
	uint16_t crcts;
	uint16_t empty;
	uint16_t crc;
} fr_get_crc_tc;

/*FASETBSTR*/
typedef struct
{
	dev_inf dev;
	uint8_t nums;
	uint8_t lens;
	uint8_t str[];
} fa_set_bstr;

/*FRSETBSTR*/
typedef struct
{
	dev_inf dev;
	uint8_t nums;
	uint8_t lens;
	uint8_t empty[3];
	uint16_t crc;
} fr_set_bstr;

/*FAGETBSTR*/
typedef struct
{
	dev_inf dev;
	uint8_t nums;
	uint32_t empty;
	uint16_t crc;
} fa_get_bstr;

/*FRGETBSTR*/
typedef struct
{
	dev_inf dev;
	uint8_t nums;
	uint8_t lens;
	uint8_t str[];
} fr_get_bstr;

/*FASETDATETIME*/
typedef struct
{
	dev_inf dev;
	uint8_t mon;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint16_t crc;
} fa_set_date_time;

/*FRSETDATETIME*/
typedef struct
{
	dev_inf dev;
	uint8_t mon;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint16_t crc;
} fr_set_date_time;

/*FAGETSTATE*/
typedef struct
{
	dev_inf dev;
	uint8_t door_open;
	uint32_t empty;
	uint16_t crc;
} fa_get_state;

/*FRGETSTATE*/
typedef struct
{
	dev_inf dev;
	uint8_t door_open;
	uint32_t empty;
	uint16_t crc;
} fr_get_state;

#pragma pack(pop)

#endif
