#include "radio_prot_h.h"
/*#include <crc16.h>*/
#if PROT_MASTER
	radio_prot *this_device;
	input_dev in_dev[MAX_DEV];
	
	uint8_t form_clpl(uint8_t lang, uint8_t point, uint8_t lvl)
	{
		if (lang > 7 || point > 7 || lvl > 3) return 0;
		return (lang << 5) | (point << 2) | (lvl);
	}
	
	dev_inf reverse_dev(dev_inf dev)
	{ //меняем местами данные устройства отправителя и приемника 
		dev_inf dev_tmp;
		dev_tmp.dev_id_from = dev.dev_id_to;
		dev_tmp.dev_sn_from = dev.dev_sn_to;
		dev_tmp.dev_id_to   = dev.dev_id_from;
		dev_tmp.dev_sn_to   = dev.dev_sn_from;
		return dev_tmp;
	}
	
	void radio_recv(uint8_t *data, size_t len)
	{
		dev_inf *dev = data;
		if (dev->func != FRHFCONTENT)
		{
			if (crc_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) return;
		}
		switch(dev->func)
		{
			case FRSEEK:
			{
				fr_seek *dev_data = data;
				if (crc_calc(data, len - 2) != dev_data->crc) break;//несоответствие контрольных сумм данных
				if (dev_data->dev.dev_id_to != this_device->dev_id) break; //получен чужой пакет
				uint8_t step = 5;
				while (step--) {
					if (dev_data->al[step])
				}
				add_ri(dev_data); //добавление нового устройства
				
			
			break;
			}
			case FRSEEKEXT:
			{
				fr_seek_ext *dev_data = data;
				if (crc_calc(data, len - 2) != dev_data->crc) break;//несоответствие контрольных сумм данных
				write_points(dev_data->al);
				add_ri(dev_data->dev);

			break;
			}
			case FRCONTENT: 
			{
				fr_content *dev_data = data;
				if (crc_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) break;//несоответствие контрольных сумм данных
				write_txt(dev_data->data, dev_data->clpl, dev_data->nftk, len - 16); //формирование файла из фрагментов
				if (++(dev_data->nftk) == dev_data->aftk) break; //пришел последний фрагмент текстового файла
				fa_content new_data;
				size_t data_len = sizeof(new_data);
				new_data.dev = reverse_dev(dev_data->dev);
				new_data.dev.func = FACONTENT;
				new_data.clpl = dev_data->clpl;
				new_data.nftk = dev_data->nftk; 
				new_data.crc = crc_calc(&new_data, data_len - 3);
				radio_send((uint8_t*)&new_data, data_len);
				
			break;
			}
			case FRHFCONTENT:
			{
				fr_hf_content *dev_data = data;
				if (crc_calc(data, len - 4) != dev_data->crc) break;//несоответствие контрольных сумм данных 
				file_check(dev_data->clpl, dev_data->hf);
				
			break;
			}
			case FRFILE:
			{
				fr_file *dev_data = data;
				if (crc_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) break;//несоответствие контрольных сумм данных
				write_file(dev_data->data, dev_data->nf, dev_data->nff, len - 16); //формирование файла из фрагментов
				if (++(dev_data->nff) == dev_data->aff) break; //пришел последний фрагмент текстового файла
				fa_file new_data;
				size_t data_len = sizeof(new_data);
				new_data.dev = reverse_dev(dev_data->dev);
				new_data.dev.func = FAFILE;
				new_data.nf = dev_data->nf;
				new_data.nff = dev_data->nff; 
				new_data.crc = crc_calc(&new_data, data_len - 3);
				radio_send((uint8_t*)&new_data, data_len);
				
			break;
			}
			case FRCALLFILTER:
			{
				fr_call_filter *dev_data = data;
				if (crc_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) break;//несоответствие контрольных сумм данных
				write_filter(dev_data->data, dev_data->nFiltra, len - 15); //формирование файла из фрагментов
				if (++(dev_data->nFiltra) == dev_data->aFiltra) break; //пришел последний фрагмент текстового файла
				fa_call_filter new_data;
				size_t data_len = sizeof(new_data);
				new_data.dev = reverse_dev(dev_data->dev);
				new_data.dev.func = FACALLFILTER;
				new_data.nFiltra = dev_data->nFiltra;
				new_data.aFiltra = dev_data->aFiltra; 
				new_data.crc = crc_calc(&new_data, data_len - 4);
				radio_send((uint8_t*)&new_data, data_len);
				
			break;
			}
			case FRSETTS:
			{
				fr_set_ts *dev_data = data;
				if (crc_calc(data, len - 2) != dev_data->crc) break;//несоответствие контрольных сумм данных
				//что-то делать
				
			break;
			}
			case FRSETPROP:
			{
				fr_set_prop *dev_data = data;
				if (crc_calc(data, len - 2) != dev_data->crc) break;//несоответствие контрольных сумм данных
				//что-то делать
				
			break;
			}
			case FRGETPROP:
			{
				fr_get_prop *dev_data = data;
				if (crc_calc(data, len - 2) != *(uint16_t*)(data + len - 2)) break;//несоответствие контрольных сумм данных
				//что-то делать
				
			break;
			}
			case FRSETCONTENT:
			{
				fr_set_content *dev_data = data;
				if (crc_calc(data, len - 2) != dev_data->crc) break;//несоответствие контрольных сумм данных
				//что-то делать
				
			break;
			}
			case FRSAVEFIRMWARE:
			{
				
				
			break;
			}
			case FRSETFILE:
			{
				break;
			}
			case FRGETCRCFILE:
			{
				break;
			}
		}
	}

	void seek_send(){
		fa_seek new_data;
		new_data.dev.dev_id_from = this_device->dev_id;
		new_data.dev.dev_sn_from = this_device->dev_sn;
		new_data.dev.dev_id_to = 0;
		new_data.dev.dev_sn_to = 0;

		new_data.capfl = this_device->cap;
	}

	void prot_init(radio_prot *dev)
		{
			this_device = dev;
			uint8_t step = 3;
			while (step--) this_device.dev_sn[step] = sn[step];
		}
#endif
