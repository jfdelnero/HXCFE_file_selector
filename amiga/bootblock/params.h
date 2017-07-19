
typedef struct ldtable_
{
	uint32_t startoffset;
	uint32_t lenght;
} ldtable;

typedef struct params_
{
	uint32_t SysBase;
	uint32_t ioreq;
	uint32_t exec_size;
	uint32_t exec_checksum;
	uint32_t total_blocks_size;
	ldtable  blocktable[4];
} params;
