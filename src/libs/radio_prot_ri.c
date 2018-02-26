#include "radio_prot_h.h"
#include <stdio.h>
/*#include <crc16.h>*/
#if !(PROT_MASTER)
	uint16_t crc16_calc(uint8_t* data, size_t len){
		return 0xFFFF;
	}
	radio_prot *this_device;

	void reverse_dev(dev_inf *lhv_dev, dev_inf rhv_dev)
	{ /*меняем местами данные устройства отправителя и приемника*/
		lhv_dev->dev_id_from = rhv_dev.dev_id_to;
		lhv_dev->dev_sn_from = rhv_dev.dev_sn_to;
		lhv_dev->dev_id_to   = rhv_dev.dev_id_from;
		lhv_dev->dev_sn_to   = rhv_dev.dev_sn_from;
	}
	void door_stat_send(uint8_t status){
		return;
	}
	void radio_receive(uint8_t *data, size_t len)
	{
		dev_inf *dev = (dev_inf*)data;
		if ((dev->dev_sn_to != this_device->dev_sn) && dev->func != FASEEK) {  /*получен чужой пакет*/
			if(DEBUG) printf("Wrong input device\n");
			return;
		}
		uint8_t prot_buff[FRAG_LEN + 16];
		size_t out_data_len = 0;
		switch(dev->func)
		{
			case FASEEK:
			{
				fa_seek *dev_data = (fa_seek*)data;
				if (crc16_calc((uint8_t*)dev, 9) != dev_data->crc){ /*несоответствие контрольных сумм хедера*/
					if(DEBUG) printf("Wrong dev crc\n");
					break;
				}
				if (crc16_calc((uint8_t*)&dev_data->lp, 3) != dev_data->crc2){  /*несоответствие контрольных сумм данных*/
					if(DEBUG) printf("Wrong pkt crc(fr_seek)\n");
					//break;
				}
				if (dev_data->lp != this_device->lp){  /*несоответствие языковой пары*/
					if(DEBUG) printf("Wrong lang pair\n");
					break;
				}

				if (dev_data->crc2 == 0) 
				{ 				
					fr_seek *new_data = (fr_seek*)prot_buff;
					out_data_len = sizeof(fr_seek);
					reverse_dev(&new_data->dev, dev_data->dev);
					new_data->dev.dev_id_from = this_device->dev_id;
					new_data->dev.dev_sn_from = this_device->dev_sn;
					new_data->dev.func = FRSEEK;
					uint8_t step = 5;
					while (step--) new_data->al[step] = this_device->al[step]; /*заполнение точек*/
					new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);
				}
				else 
				{
					fr_seek_ext *new_data = (fr_seek_ext*)prot_buff;
					reverse_dev(&new_data->dev, dev_data->dev);
					new_data->dev.dev_id_from = this_device->dev_id;
					new_data->dev.dev_sn_from = this_device->dev_sn;
					new_data->dev.func = FRSEEKEXT;
					uint8_t step = 5; 
					while (step--) new_data->al[step] = this_device->al[step];
					new_data->lp = this_device->lp;
					if (dev_data->capfl > 255){
						if (this_device->ap < dev_data->capfl 
							|| (this_device->ap - this_device->h) > dev_data->capfl)
						{
							if (DEBUG) printf("Device out of ap range\n");
							break;
						}
					}else if (this_device->fl != (uint8_t)dev_data->capfl)
					{
						if (DEBUG) printf("Device on wrong floor\n");
						break;
					}
					out_data_len = sizeof(fr_seek_ext);
					new_data->ap = this_device->ap;
					new_data->h = this_device->h;
					new_data->fl = this_device->fl;
					new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);
				}
				break;
			}
			case FACONTENT:
			{
				fa_content *dev_data = (fa_content*)data;
				if (crc16_calc(data, len - 2) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_content)\n");
					break;
				}
				fr_content *new_data = (fr_content*)prot_buff;
				out_data_len = sizeof(fr_content);
				size_t c_len = 0;
				get_content(dev_data->clpl, &c_len, dev_data->nftk, new_data->data);
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
				get_hf_content(dev_data->clpl, &new_data->hf);	/*формирование хэш-функции*/
				while(out_data_len % 8) prot_buff[out_data_len++ - 2] = 0;
				*(uint16_t*)(prot_buff + out_data_len - 2) = crc16_calc(prot_buff, out_data_len - 2);
				
				break;
			}
			case FAHFCONTENT:
			{
				fa_hf_content *dev_data = (fa_hf_content*)data;
				if (crc16_calc(data, len - 6) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_hf_content)\n");
					break;
				}
				fr_hf_content *new_data = (fr_hf_content*)prot_buff;
				out_data_len = sizeof(fr_hf_content);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRHFCONTENT;
				new_data->clpl = dev_data->clpl;
				get_hf_content(dev_data->clpl, &new_data->hf);
				new_data->crc = crc16_calc((uint8_t*)new_data, sizeof(new_data) - 4);
				
				break;
			}
			case FAFILE:
			{		
				fa_file *dev_data = (fa_file*)data;
				if (crc16_calc(data, len - 2) != dev_data->crc) {
					if (DEBUG) printf("Wrong pkt crc(fr_file)\n");
					break;
				}
				fr_file *new_data = (fr_file*)prot_buff;
				out_data_len = sizeof(fr_file);
				size_t f_len = 0;
				get_file(dev_data->nf, &f_len, dev_data->nff, new_data->data);
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
				*(uint16_t*)(prot_buff + out_data_len - 2) = crc16_calc(prot_buff, out_data_len - 2);
				
				break;
			}
			case FACALLFILTER:
			{		
				fa_call_filter *dev_data = (fa_call_filter*)data;
				if (crc16_calc(data, len - 4) != dev_data->crc) break;
				fr_call_filter *new_data = (fr_call_filter*)prot_buff;
				out_data_len = sizeof(fr_call_filter);
				//if (dev_data->np == 5) alarm(); /*по протоколу должно быть, но нигде не реализовано*/
				if (dev_data->np == 255) break;
				size_t f_len = 0;
				get_filter(dev_data->l, dev_data->np,  &f_len, new_data->nFiltra, new_data->data);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRFILE;
				new_data->l = dev_data->l;
				new_data->np = dev_data->np;
				new_data->nFiltra = dev_data->nFiltra;
				new_data->aFiltra = f_len / FRAG_LEN;
				if (f_len % FRAG_LEN) ++new_data->aFiltra;
				if ((new_data->nFiltra + 1) == new_data->aFiltra)
				{
					out_data_len += f_len % FRAG_LEN + 2;
				}else
				{
					out_data_len += FRAG_LEN + 2;
				}
				while(out_data_len % 8) prot_buff[out_data_len++ - 2] = 0;
				*(uint16_t*)(prot_buff + out_data_len - 2) = crc16_calc(prot_buff, out_data_len - 2);
				
				break;
			}
			case FASETTS:
			{
				fa_set_ts *dev_data = (fa_set_ts*)data;
				if (crc16_calc(data, len - 2) != dev_data->crc) break;
				if (dev->dev_id_from != 8) break;
				fr_set_ts *new_data = (fr_set_ts*)prot_buff;
				out_data_len = sizeof(fr_set_ts);
				if (dev_data->tts == 255)
				{
					reverse_dev(&new_data->dev, dev_data->dev);
					new_data->dev.func = FRSETTS;
					get_ts(&new_data->tts, &new_data->nts, &new_data->lts, &new_data->dir);
					new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);
				} else 
				{
					reverse_dev(&new_data->dev, dev_data->dev);
					new_data->dev.func = FRSETTS;
					new_data->tts = dev_data->tts;
					new_data->nts = dev_data->nts;
					new_data->lts = dev_data->lts;
					new_data->dir = dev_data->dir;
					set_ts(dev_data->tts, dev_data->nts, dev_data->lts, dev_data->dir);
					new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);
				}
				
				
				break;
			}
			case FASETPROP:
			{		
				fa_set_prop *dev_data = (fa_set_prop*)data;
				if (crc16_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) break;
				if (dev->dev_id_from != 8) break;
				fr_set_prop *new_data = (fr_set_prop*)prot_buff;
				out_data_len = sizeof(fr_set_prop);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETPROP;
				new_data->nfp = dev_data->nfp;
				new_data->qp = dev_data->qp;
				new_data->er = set_params(new_data->nfp, new_data->qp, dev_data->vp);
				new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);

				break;
			}
			case FAGETPROP:
			{	
				fa_get_prop *dev_data = (fa_get_prop*)data;
				if (crc16_calc(data, len - 2) != dev_data->crc) break;
				if (dev->dev_id_from != 8) break;
				fr_get_prop *new_data = (fr_get_prop*)prot_buff;
				out_data_len = sizeof(fr_get_prop);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRGETPROP;
				new_data->nfp = dev_data->nfp;
				new_data->qp = dev_data->qp;
				get_params(new_data->nfp, new_data->qp, new_data->vp);
				while(out_data_len % 8) prot_buff[out_data_len++ - 2] = 0;
				*(uint16_t*)(prot_buff + out_data_len - 2) = crc16_calc(prot_buff, out_data_len - 2);
				
				break;
			}
			case FASETCONTENT:
			{	
				fa_set_content *dev_data = (fa_set_content*)data;
				if (crc16_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) break;
				if (dev->dev_id_from != 8) break;
				fr_set_content *new_data = (fr_set_content*)prot_buff;
				out_data_len = sizeof(fr_set_content);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETPROP;
				new_data->clpl = dev_data->clpl;
				new_data->nftk = dev_data->nftk;
				set_content(new_data->clpl, new_data->nftk, dev_data->lenFr, dev_data->data);
				new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);

				break;
			}
			/*case FASAVEFIRMWARE:
			{	
				fa_save_firmware *dev_data = (fa_save_firmware*)data;
				if (crc16_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) break;
				if (dev->dev_id_from != 8) break;
				fr_save_firmware *new_data = (fr_save_firmware*)prot_buff;
				out_data_len = sizeof(fr_save_firmware);
				uint8_t* firmware = dev_data->data;
				size_t firmware_len = len - 16; //РґР»РёРЅР° С„СЂР°РіРјРµРЅС‚Р° РїСЂРѕС€РёРІРєРё
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data.dev.func = FRSAVEFIRMWARE;
				new_data.clpl = dev_data->clpl;
				new_data.nftk = dev_data->nftk;
				set_content(content, content_len, new_data.clpl, new_data.nftk);
				new_data.crc = crc16_calc(&new_data, data_len - 2);
				
				break;
			}*/
			case FASETFILE:
			{	
				fa_set_file *dev_data = (fa_set_file*)data;
				if (crc16_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) break;
				if (dev->dev_id_from != 8) break;
				fr_set_file *new_data = (fr_set_file*)prot_buff;
				out_data_len = sizeof(fr_set_file);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETFILE;
				new_data->nf = dev_data->nf;
				new_data->nff = dev_data->nff;
				if (dev_data->lenFr == 0xFF || dev_data->lenFr > 128) new_data->lenFr = 128;
				else if(dev_data->lenFr <= 128) new_data->lenFr = dev_data->lenFr;
				set_file(new_data->nf, new_data->nff, new_data->lenFr, dev_data->data);
				new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);
				
				break;
			}
			case FAGETCRCFILE:
			{	
				fa_get_crc_file *dev_data = (fa_get_crc_file*)data;
				if (crc16_calc(data, len - 6) != dev_data->crc) break;
				if (dev->dev_id_from != 8) break;
				fr_get_crc_file *new_data = (fr_get_crc_file*)prot_buff;
				out_data_len = sizeof(fr_get_crc_file);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRGETCRCFILE;
				new_data->nf = dev_data->nf;
				/*new_data->crcf = crc16_calc(file, file_len);*/
				new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);
				
				break;
			}
			case FAGETCRCTC:
			{
				fa_get_crc_tc *dev_data = (fa_get_crc_tc*)data;
				if (crc16_calc(data, len - 6) != dev_data->crc) break;
				if (dev->dev_id_from != 8) break;
				fr_get_crc_tc *new_data = (fr_get_crc_tc*)prot_buff;
				out_data_len = sizeof(fr_get_crc_tc);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRGETCRCTC;
				get_hf_content(dev_data->ktu, &new_data->crcts);
				new_data->ktu = dev_data->ktu;
				new_data->crc = crc16_calc((uint8_t*)new_data, out_data_len - 2);
				break;
			}
			/*case FASETBSTR:
			{
				fa_set_bstr *dev_data = (fa_set_bstr*)data;
				if (crc16_calc(data, len - 6) != dev_data->crc) break;
				fr_set_bstr *new_data = (fr_set_bstr*)prot_buff;
				out_data_len = sizeof(fr_set_bstr);
				reverse_dev(&new_data->dev, dev_data->dev);
				new_data->dev.func = FRSETBSTR;
				new_data->nums = dev_data->nums;
				break;
			}*/
			case FAGETSTATE:
			{
				/*send_door_status(dev);*/
				break;
			}
		}
		if (out_data_len == 0) return;
		radio_send(prot_buff, out_data_len);
	}
	void prot_init(radio_prot *dev)
	{
		this_device = dev;
		dev->radio_recv = &radio_receive;
		dev->door_status =&door_stat_send;
	}
#endif
