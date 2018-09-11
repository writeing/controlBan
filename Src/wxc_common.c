#include "wxc_common.h"

static char s_arrFlag[30] = {0};

void setArrFlag(int index)
{
	s_arrFlag[index] = WXC_OK;
}
void clearArrFlag(int index)
{
	s_arrFlag[index] = WXC_ERROR;
}
int getArrFlag(int index)
{
	return s_arrFlag[index];
}
























