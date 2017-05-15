#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
int main()
{
OpenSSL_add_all_algorithms();
}

