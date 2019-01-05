#ifndef __PARTHLP_H__
#define __PARTHLP_H__

/*
	[IN] nDrive - номер HDD 0..3,
	[IN] nPartition - номер раздела на диске - 0..(N-1)
	[OUT] cLetter - 'A' -'Z', если раздел смонтирован, иначе 0;
	RETURN: 1 - OK (cLetter is valid),
			0 - ERROR
*/

int GetPartitionLetter(unsigned int nDrive, unsigned int nPartition, CHAR &cLetter);

#endif //__PARTHLP_H__