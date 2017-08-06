#include <common.h>
#include <sys/types.h>

void * memset( void * dest, BYTE val, int count )
{
    register char * d = (char *)dest;
    while( count-- )
    	*d++ = val;
    return dest;
}
DWORD strlen(const char* str)
{
	DWORD ret = 0;
	while ( str[ret] != 0 )
		ret+=1;
	return ret;
}
void
strrev(unsigned char *str)
{
        int i;
        int j;
        unsigned char a;
        unsigned len = strlen((const char *)str);

        for (i = 0, j = len - 1; i < j; i++, j--)
        {
                a = str[i];
                str[i] = str[j];
                str[j] = a;
        }
}
int
itoa(int num, unsigned char* str, int len, int base)
{
        int sum = num;
        int i = 0;
        int digit;

        if (len == 0)
                return -1;

        do
        {
                digit = sum % base;

                if (digit < 0xA)
                        str[i++] = '0' + digit;
                else
                        str[i++] = 'A' + digit - 0xA;

                sum /= base;

        }while (sum && (i < (len - 1)));

        if (i == (len - 1) && sum)
                return -1;

        str[i] = '\0';
        strrev(str);

        return 0;
}
