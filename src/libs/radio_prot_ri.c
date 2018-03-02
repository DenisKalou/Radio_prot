#include "radio_prot_h.h"
#include <stdio.h>
/*#include <crc16.h>*/
#if !(PROT_MASTER)
	static uint8_t prot_buff[FRAG_LEN + 16];
	static protRadio this_device;
	static uint8_t door_status = 0;
	protRadio *prot_radio = &this_device;

	void reverse_dev(dev_inf *lhv_dev, dev_inf rhv_dev)
	{ /*меняем местами данные устройства отправителя и приемника*/
		lhv_dev->dev_id_to   = rhv_dev.dev_id_from;
		lhv_dev->dev_sn_to   = rhv_dev.dev_sn_from;
	}
	uint8_t device_data_convert(dev_inf *dev){
		if(this_device.hwRadioAAL.get_device_data == NULL) return 1;
		uint8_t id;
		uint32_t sn;
		this_device.hwRadioAAL.get_device_data(&id, &sn);
		dev->dev_id_from = id;
		dev->dev_sn_from = sn;
		return 0;
	}
	void door_stat_send(uint8_t status){
		door_status = status;
		if(DEBUG) printf("stat = %d\n", door_status);
		fr_get_state *new_data = (fr_get_state*)prot_buff;
		dev_inf *tdev = (dev_inf*)prot_buff;
		if(device_data_convert(tdev)) return;
		new_data->dev.dev_id_to = 0;
		new_data->dev.dev_sn_to = 0;
		new_data->dev.func = FRGETSTATE;
		new_data->door_open = door_status;
		new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)&new_data, sizeof(fr_get_state) - 2);
		radio_send((uint8_t*)&new_data, sizeof(fr_get_state));
	}
	void radio_receive(uint8_t *data, size_t len)
	{

		dev_inf *dev = (dev_inf*)data;
		dev_inf *tdev = (dev_inf*)prot_buff;
		if(device_data_convert(tdev)) return;
		if ((dev->dev_sn_to != tdev->dev_sn_from) && dev->func != FASEEK) {  /*получен чужой пакет*/
			if(DEBUG) printf("Wrong input device\n");
			return;
		}
		size_t out_data_len = 0;
		switch(dev->func)
		{
			case FASEEK:
			{
				fa_seek *dev_data = (fa_seek*)data;
				if (this_device.hwRadioAAL.crc16_calc((uint8_t*)&dev_data->dev, 9) != dev_data->crc){ /*несоответствие контрольных сумм хедера*/
					if(DEBUG) printf("Wrong dev crc\n");
					break;
				}
				if (this_device.hwRadioAAL.crc16_calc((uint8_t*)&dev_data->lp, 3) != dev_data->crc2){  /*несоответствие контрольных сумм данных*/
					if(DEBUG) printf("Wrong pkt crc(fr_seek)\n");
					//break;
				}
				if (dev_data->crc2 == 0) 
				{ 				
					fr_seek *new_data = (fr_seek*)prot_buff;
					out_data_len = sizeof(fr_seek);
					reverse_dev(&new_data->dev, dev_data->dev);
					new_data->dev.func = FRSEEK;
					uint16_t dummy;
					this_device.hwRadioAAL.get_params_data(new_data->al, (uint8_t*)&dummy, &dummy, (uint8_t*)&dummy, (uint8_t*)&dummy);
					new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				}
				else 
				{
					fr_seek_ext *new_data = (fr_seek_ext*)prot_buff;
					reverse_dev(&new_data->dev, dev_data->dev);
					this_device.hwRadioAAL.get_params_data(new_data->al, &new_data->lp,
							&new_data->ap, &new_data->h, &new_data->fl);
					if (dev_data->lp != new_data->lp){  /*несоответствие языковой пары*/
						if(DEBUG) printf("Wrong lang pair\n");
						break;
					}
					new_data->dev.func = FRSEEKEXT;
					if (dev_data->capfl > 255){
						if (new_data->ap < dev_data->capfl
							|| (new_data->ap - new_data->h) > dev_data->capfl)
						{
							if (DEBUG) printf("Device out of ap range\n");
							break;
						}
					}else if (new_data->fl != (uint8_t)dev_data->capfl)
					{
						if (DEBUG) printf("Device on wrong floor\n");
						break;
					}
					out_data_len = sizeof(fr_seek_ext);
					new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				}
				break;
			}
			case FACONTENT:
			{
				fa_content *dev_data = (fa_content*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_content)\n");
					break;
				}
				fr_content *new_data = (fr_content*)prot_buff;
				out_data_len = sizeof(fr_content);
				size_t c_len = 0;
				this_device.hwRadioAAL.get_content(dev_data->clpl, &c_len, dev_data->nftk, new_data->data);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRCONTENT;
				new_data->clpl = dev_data->clpl;
				new_data->nftk = dev_data->nftk;
				new_data->aftk = c_len / FRAG_LEN;
				if (c_len % FRAG_LEN) ++new_data->aftk;
				if ((new_data->nftk + 1) > new_data->aftk)
				{
					if (DEBUG) printf("No such fragment(fr_content)\n");
					out_data_len = 0;
					break;
				}else if ((new_data->nftk + 1) == new_data->aftk)
				{
					out_data_len += c_len % FRAG_LEN + 2;
				}else
				{
					out_data_len += FRAG_LEN + 2;
				} 
				this_device.hwRadioAAL.get_hf_content(dev_data->clpl, &new_data->hf);	/*формирование хэш-функции*/
				while(out_data_len % 8) prot_buff[out_data_len++ - 2] = 0;
				*(uint16_t*)(prot_buff + out_data_len - 2) = this_device.hwRadioAAL.crc16_calc(prot_buff, out_data_len - 2);
				
				break;
			}
			case FAHFCONTENT:
			{
				fa_hf_content *dev_data = (fa_hf_content*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 6) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_hf_content)\n");
					break;
				}
				fr_hf_content *new_data = (fr_hf_content*)prot_buff;
				out_data_len = sizeof(fr_hf_content);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRHFCONTENT;
				new_data->clpl = dev_data->clpl;
				this_device.hwRadioAAL.get_hf_content(dev_data->clpl, &new_data->hf);
				new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, sizeof(new_data) - 4);
				
				break;
			}
			case FAFILE:
			{		
				fa_file *dev_data = (fa_file*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_file)\n");
					break;
				}
				fr_file *new_data = (fr_file*)prot_buff;
				out_data_len = sizeof(fr_file);
				size_t f_len = 0;
				this_device.hwRadioAAL.get_file(dev_data->nf, &f_len, dev_data->nff, new_data->data);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRFILE;
				new_data->nf = dev_data->nf;
				new_data->nff = dev_data->nff;
				new_data->aff = f_len / WAV_LEN;
				if (f_len % WAV_LEN) ++new_data->aff;
				if ((new_data->nff + 1) > new_data->aff)
				{
					if (DEBUG) printf("No such fragment(fr_file)\n");
					out_data_len = 0;
					break;
				}else if ((new_data->nff + 1) == new_data->aff)
				{
					out_data_len += f_len % WAV_LEN + 2;
				}else
				{
					out_data_len += WAV_LEN + 2;
				}
				while(out_data_len % 8) prot_buff[out_data_len++ - 2] = 0;
				*(uint16_t*)(prot_buff + out_data_len - 2) = this_device.hwRadioAAL.crc16_calc(prot_buff, out_data_len - 2);
				
				break;
			}
			case FACALLFILTER:
			{		
				fa_call_filter *dev_data = (fa_call_filter*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 4) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_call_filter)\n");
					break;
				}
				if (dev_data->np > 5) {
					if (DEBUG) printf("DO NOT CALL FILTER\n");
					break;
				}
				fr_call_filter *new_data = (fr_call_filter*)prot_buff;
				out_data_len = sizeof(fr_call_filter);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRCALLFILTER;
				new_data->l = dev_data->l;
				size_t f_len = 0;
				if (this_device.hwRadioAAL.call_filter(dev_data->l, dev_data->np,
						&f_len, dev_data->nFiltra, new_data->data)){
					new_data->np = 0xFF;
					new_data->nFiltra = 0;
					new_data->aFiltra = 0;
					out_data_len += 3;
					*(uint16_t*)(prot_buff + out_data_len - 2)
							= this_device.hwRadioAAL.crc16_calc(prot_buff, out_data_len - 2);
					break;
				}else if (dev_data->np == 5){
					new_data->np = dev_data->np;
					new_data->nFiltra = 0;
					new_data->aFiltra = 0;
					out_data_len += 3;
					*(uint16_t*)(prot_buff + out_data_len - 2)
						= this_device.hwRadioAAL.crc16_calc(prot_buff, out_data_len - 2);
					break;
				}
				new_data->np = dev_data->np;
				new_data->nFiltra = dev_data->nFiltra;
				new_data->aFiltra = f_len / FRAG_LEN;
				if (f_len % FRAG_LEN) ++new_data->aFiltra;
				if ((new_data->nFiltra + 1) < new_data->aFiltra)
				{
					out_data_len += FRAG_LEN + 2;
				}else if ((new_data->nFiltra + 1) == new_data->aFiltra)
				{
					out_data_len += f_len % FRAG_LEN + 2;
				} else {
					if (DEBUG) printf("No such fragment(fr_call_filter)\n");
					out_data_len = 0;
					break;
				}
				while(out_data_len % 8) prot_buff[out_data_len++ - 2] = 0;
				*(uint16_t*)(prot_buff + out_data_len - 2)
						= this_device.hwRadioAAL.crc16_calc(prot_buff, out_data_len - 2);
				break;
			}
			case FASETTS:
			{
				fa_set_ts *dev_data = (fa_set_ts*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_set_ts)\n");
					break;
				}
				if (dev->dev_id_from != 8) {
					if (DEBUG) printf("Its not radio setter(fr_set_ts)\n");
					break;
				}
				fr_set_ts *new_data = (fr_set_ts*)prot_buff;
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETTS;
				out_data_len = sizeof(fr_set_ts);
				if (dev_data->tts == 255)
				{
					this_device.hwRadioAAL.get_ts(&new_data->tts, &new_data->nts, &new_data->lts, &new_data->dir);
					new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				} else 
				{
					new_data->tts = dev_data->tts;
					new_data->nts = dev_data->nts;
					new_data->lts = dev_data->lts;
					new_data->dir = dev_data->dir;
					this_device.hwRadioAAL.set_ts(dev_data->tts, dev_data->nts, dev_data->lts, dev_data->dir);
					new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				}
				
				
				break;
			}
			case FASETPROP:
			{		
				fa_set_prop *dev_data = (fa_set_prop*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) {
					if (DEBUG) printf("Wrong pkt crc(fr_set_prop)\n");
					break;
				}
				if (dev->dev_id_from != 8) {
					if (DEBUG) printf("Its not radio setter(fr_set_prop)\n");
					break;
				}
				fr_set_prop *new_data = (fr_set_prop*)prot_buff;
				out_data_len = sizeof(fr_set_prop);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETPROP;
				new_data->nfp = dev_data->nfp;
				new_data->qp = dev_data->qp;
				new_data->er = this_device.hwRadioAAL.set_params(new_data->nfp, new_data->qp, dev_data->vp);
				new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);

				break;
			}
			case FAGETPROP:
			{	
				fa_get_prop *dev_data = (fa_get_prop*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != dev_data->crc){
					if (DEBUG) printf("Wrong pkt crc(fr_get_prop)\n");
					break;
				}
				if (dev->dev_id_from != 8){
					if (DEBUG) printf("Its not radio setter(fr_get_prop)\n");
					break;
				}
				fr_get_prop *new_data = (fr_get_prop*)prot_buff;
				out_data_len = sizeof(fr_get_prop);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRGETPROP;
				new_data->nfp = dev_data->nfp;
				new_data->qp = dev_data->qp;
				this_device.hwRadioAAL.get_params(new_data->nfp, new_data->qp, new_data->vp);
				out_data_len += new_data->qp * 2 + 2;
				while(out_data_len % 8) prot_buff[out_data_len++ - 2] = 0;
				*(uint16_t*)(prot_buff + out_data_len - 2) = this_device.hwRadioAAL.crc16_calc(prot_buff, out_data_len - 2);
				
				break;
			}
			case FASETCONTENT:
			{	
				fa_set_content *dev_data = (fa_set_content*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) {
					if (DEBUG) printf("Wrong pkt crc(fr_set_content)\n");
					break;
				}
				if (dev->dev_id_from != 8) {
					if (DEBUG) printf("Its not radio setter(fr_set_content)\n");
					break;
				}
				fr_set_content *new_data = (fr_set_content*)prot_buff;
				out_data_len = sizeof(fr_set_content);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETPROP;
				new_data->clpl = dev_data->clpl;
				new_data->nftk = dev_data->nftk;
				this_device.hwRadioAAL.set_content(new_data->clpl, new_data->nftk, dev_data->lenFr, dev_data->data);
				new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);

				break;
			}
			case FASAVEFIRMWARE://добавить проверку по длине фрагмента(последнего), проверку версии при записи
			{	
				fa_save_firmware *dev_data = (fa_save_firmware*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) {
					if (DEBUG) printf("Wrong pkt crc(fr_save_firmware)\n");
					break;
				}
				if (dev->dev_id_from != 8) {
					if (DEBUG) printf("Its not radio setter(fr_save_firmware)\n");
					break;
				}
				fr_save_firmware *new_data = (fr_save_firmware*)prot_buff;
				out_data_len = sizeof(fr_save_firmware);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSAVEFIRMWARE;
				new_data->flags = 0;
				new_data->flags |= (dev_data->lf & 1);
				this_device->hwRadioAAL.firmware_check(&new_data->v, &new_data->nf);
				if (dev_data->nf == 0){
					new_data->flags |= (1 << FFW);
					if(this_device->hwRadioAAL.save_firmware(dev_data->nf,
							(dev_data->lf & 1), &dev_data->data)) {
						new_data->flags |= (1 << ER_FFW);
					} else new_data->nf = dev_data->nf;
				} else if (dev_data->nf > 0){
					if(this_device->hwRadioAAL.save_firmware(dev_data->nf,
							(dev_data->lf & 1), &dev_data->data)) {
						new_data->flags |= (1 << ER_CHECK);
					} else new_data->nf = dev_data->nf;
				}
				new_data->crc = this_device->hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				break;
			}
			case FASETFILE:
			{	
				fa_set_file *dev_data = (fa_set_file*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) {
					if (DEBUG) printf("Wrong pkt crc(fr_set_file)\n");
					break;
				}
				if (dev->dev_id_from != 8) {
					if (DEBUG) printf("Its not radio setter(fr_set_file)\n");
					break;
				}
				fr_set_file *new_data = (fr_set_file*)prot_buff;
				out_data_len = sizeof(fr_set_file);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETFILE;
				new_data->nf = dev_data->nf;
				new_data->nff = dev_data->nff;
				if (dev_data->lenFr == 0xFF || dev_data->lenFr > 128) new_data->lenFr = 128;
				else if(dev_data->lenFr <= 128) new_data->lenFr = dev_data->lenFr;
				this_device.hwRadioAAL.set_file(new_data->nf, new_data->nff, new_data->lenFr, dev_data->data);
				new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				
				break;
			}
			case FAGETCRCFILE:
			{	
				fa_get_crc_file *dev_data = (fa_get_crc_file*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 6) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_get_crc_file)\n");
					break;
				}
				if (dev->dev_id_from != 8) {
					if (DEBUG) printf("Its not radio setter(fr_get_crc_file)\n");
					break;
				}
				fr_get_crc_file *new_data = (fr_get_crc_file*)prot_buff;
				out_data_len = sizeof(fr_get_crc_file);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRGETCRCFILE;
				new_data->nf = dev_data->nf;
				this_device.hwRadioAAL.get_crc_file(new_data->nf, &new_data->crcf);
				new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				
				break;
			}
			case FAGETCRCTC:
			{
				fa_get_crc_tc *dev_data = (fa_get_crc_tc*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 6) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_get_crc_tc)\n");
					break;
				}
				if (dev->dev_id_from != 8) {
					if (DEBUG) printf("Its not radio setter(fr_get_crc_tc)\n");
					break;
				}
				fr_get_crc_tc *new_data = (fr_get_crc_tc*)prot_buff;
				out_data_len = sizeof(fr_get_crc_tc);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRGETCRCTC;
				this_device.hwRadioAAL.get_hf_content(dev_data->ktu, &new_data->crcts);
				new_data->ktu = dev_data->ktu;
				new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				break;
			}
			case FASETBSTR:
			{
				fa_set_bstr *dev_data = (fa_set_bstr*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 6) != *(uint16_t*)(data + len - 2)){
					if (DEBUG) printf("Wrong pkt crc(fr_set_bstr)\n");
					break;
				}
				fr_set_bstr *new_data = (fr_set_bstr*)prot_buff;
				if (this_device.hwRadioAAL.set_str(dev_data->nums, dev_data->lens, dev_data->str)) {
					new_data->lens = 0;
				} else {
					new_data->lens = dev_data->lens;
				}
				out_data_len = sizeof(fr_set_bstr);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETBSTR;
				new_data->nums = dev_data->nums;
				new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				break;
			}
			case FAGETBSTR:
			{
				fa_get_bstr *dev_data = (fa_get_bstr*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 6) != dev_data->crc){
					if (DEBUG) printf("Wrong pkt crc(fr_get_bstr)\n");
					break;
				}
				fr_get_bstr *new_data = (fr_get_bstr*)prot_buff;
				size_t str_len;
				this_device.hwRadioAAL.get_str(dev_data->nums, &str_len, new_data->str);
				out_data_len = sizeof(fr_get_bstr) + str_len + 2;
				reverse_dev(&new_data->dev, dev_data->dev);
				while(out_data_len % 8) prot_buff[out_data_len++ - 2] = 0;
				new_data->dev.func = FRSETBSTR;
				new_data->nums = dev_data->nums;
				new_data->lens = str_len;
				*(uint16_t*)(prot_buff + out_data_len - 2) = this_device.hwRadioAAL.crc16_calc(prot_buff, out_data_len - 2);
				break;
			}
			case FASETDATETIME:
			{
				fa_set_date_time *dev_data = (fa_set_date_time*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != dev_data->crc) {
				if (DEBUG) printf("Wrong pkt crc(fr_set_ts)\n");
					break;
				}
				if (dev->dev_id_from != 8) {
					if (DEBUG) printf("Its not radio setter(fr_set_ts)\n");
					break;
				}
				fa_set_date_time *new_data = (fa_set_date_time*)prot_buff;
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETDATETIME;
				out_data_len = sizeof(fa_set_date_time);
				if (dev_data->mon == 255)
				{
					this_device.hwRadioAAL.get_date_time(&new_data->mon, &new_data->day, &new_data->hour,
							&new_data->min, &new_data->sec);
					new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				} else
				{
					new_data->mon = dev_data->mon;
					new_data->day = dev_data->day;
					new_data->hour = dev_data->hour;
					new_data->min = dev_data->min;
					new_data->sec = dev_data->sec;
					this_device.hwRadioAAL.set_date_time(new_data->mon, new_data->day, new_data->hour,
							new_data->min, new_data->sec);
					new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				}
				break;
			}
			case FAGETSTATE:
			{
				fa_get_state *dev_data = (fa_get_state*)data;
				if (this_device.hwRadioAAL.crc16_calc(data, len - 2) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_get_state)\n");
					break;
				}
				fr_get_state *new_data = (fr_get_state*)prot_buff;
				out_data_len = sizeof(fr_get_state);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRGETSTATE;
				new_data->door_open = door_status;
				new_data->crc = this_device.hwRadioAAL.crc16_calc((uint8_t*)new_data, out_data_len - 2);
				break;
			}
		}
		if (out_data_len == 0 || this_device.radio_send == NULL) return;
		this_device.radio_send(prot_buff, out_data_len);
	}

	static protRadio this_device = {
			.hwRadioAAL.crc16_calc = NULL,
			.hwRadioAAL.get_device_data = NULL,
			.hwRadioAAL.firmware_check = NULL,
			.hwRadioAAL.get_file = NULL,
			.hwRadioAAL.set_file = NULL,
			.hwRadioAAL.get_crc_file = NULL,
			.hwRadioAAL.get_content = NULL,
			.hwRadioAAL.get_hf_content = NULL,
			.hwRadioAAL.set_content = NULL,
			.hwRadioAAL.call_filter = NULL,
			.hwRadioAAL.set_filter = NULL,
			.hwRadioAAL.save_firmware = NULL,
			.hwRadioAAL.get_ts = NULL,
			.hwRadioAAL.set_ts = NULL,
			.hwRadioAAL.get_params = NULL,
			.hwRadioAAL.set_params = NULL,
			.send_status = &door_stat_send,
			.radio_send = NULL,
			.radio_recv = &radio_receive,
	};
#endif
