// lower id -> Higher priority
//
#include <esp_err.h>


#define DTYPE_BOOL 1	
#define	DTYPE_INT 4 
#define DTYPE_F32 32
#define DTYPE_F64 64
#define DTYPE_F128 128
#define DTYPE_F256 256



typedef struct {
	char* data;
	unsigned int lenth;
}record;

typedef struct {
	record sen_name;
	int last_record_time; // In seconds
	void* buffer;
} sensor;

struct sensors {

};



void reg_new_sensor(struct sensor* sensors, sensor* sen)
{
	
}



