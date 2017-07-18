typedef struct _ldtable
{
	uint32_t startoffset;
	uint32_t lenght;
} __attribute__ ((packed)) ldtable;

typedef struct _params
{
	uint32_t exec_size;
	uint32_t exec_checksum;
	uint32_t total_nb_block;
	ldtable  blocktable[4];
} __attribute__ ((packed)) params;

